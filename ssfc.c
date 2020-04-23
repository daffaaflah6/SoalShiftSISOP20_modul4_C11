#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/bela/shift4";
char list[100]="9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char enc1[10]="/encv1_";
char enc2[10]="/encv2_";

void enkrip(char src[10000]){
    if(!strcmp(src,".") || !strcmp(src,"..")) 
    return;
    
    int enc = strlen(src);

    int k = 10;
    int h;

    for(int i=0; i<enc; i++){
        for(int j=0;j<87;j++){
            if(src[i]=='/') continue;
            if(list[j]==src[i]){
                h=(j+k)%87;
                src[i]=list[h];
                break;
            }

        }
    }
}

int enk(char src[10000]){
    DIR *dp;
    struct dirent *d;
    char name[100000];
    char path[10000], path1[10000], path2[100000];

    dp = opendir(src);
    if(dp == NULL){
        return -errno;
    }

    while ((d=readdir(dp)) != NULL){
        struct stat stat;
        memset(&stat, 0, sizeof(stat));

        strcpy(name, d->d_name);
        sprintf(path, "%s/%s", src, name);

        if(!strcmp(name, ".") && !strcmp(name, "..") && d->d_type == DT_DIR){
            char folder[10000];
            char path3[10000];

            strcpy(path3, path);
            strcpy(folder, name);
            enkrip(folder);

            sprintf(path1, "%s/%s", path, folder);
            int res = rename(path1, path2);
            if(res!=0){
                return -errno;
            }
        }

        if(d->d_type == DT_REG){
            char *ext;

            ext = strrchr(name, '.');

            if(ext == NULL){
                enkrip(name);
                sprintf(path2, "%s/%s", path, name);
            } else {
                int z = strlen(ext);
                size_t n = sizeof(name)/sizeof(name[0]);
                
                int noext = z-n+1;
                char noname[10000];

                snprintf(noname, noext, "%s", name);
                enkrip(noname);

                sprintf(path2, "%s/%s%s", path, noname, ext);
            }

            int res = rename(path, path2);
            if(res!=0){
                return -errno;
            }
        }
    }

    closedir(dp);
    return 0;

}

void dekrip(char src[1000]){
    if(!strcmp(src,".") || !strcmp(src,"..")) 
    return;
    
    int dec = strlen(src);

    int k = 77, h;

    for(int i=0; i<dec; i++){
        for(int j=0;j<87;j++){
            if(src[i]=='/') continue;
            if(list[j]==src[i]){
            h=(j+k)%87;
            src[i]=list[h];
            break;
            }

        }
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf){
	int res;
    char fpath[1000];

    sprintf(fpath,"%s%s", dirpath, path);

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi){

    char fpath[1000];
    char name[1000];

    if (strcmp(path, "/") == 0){
		sprintf(fpath, "%s", dirpath);
	} else {
		sprintf(name,"%s", path);
		sprintf(fpath, "%s%s", dirpath, name);
	}
    
    int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode){
	int res;

	char fpath[1000];
    char name[1000];

    sprintf(fpath,"%s%s", dirpath, path);

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to){
	int res;
    char fpath[1000];
    char name[1000];

    if(strncmp(to, enc1, 7) == 0){
        sprintf(name, "%s", to);
        enk(name);
        sprintf(fpath, "%s%s", dirpath, name);
    }

	res = rename(from, name);
	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.rename		= xmp_rename,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}


