#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/syarif/sisop20/modul4/shift_modul4";
char list[100]="9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char enc1[10]="/encv1_";
char enc2[10]="/encv2_";
static const char *logsys = "/home/syarif/fs.log";

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

int enk(char *src){
    DIR *dp;
    struct dirent *d;
    char name[100];
    char path[1000], path1[1000]; 
    char path2[3000];

    dp = opendir(src);
    if(dp == NULL){
        return -errno;
    }

    while ((d=readdir(dp)) != NULL){
        struct stat stat;
        memset(&stat, 0, sizeof(stat));

        strcpy(name, d->d_name);
        sprintf(path, "%s/%s", src, name);

        if(strcmp(name, ".") && strcmp(name, "..") && d->d_type == DT_DIR){
            char folder[1000];

            strcpy(folder, name);
            enkrip(folder);

            strcpy(path1, path);
            sprintf(path2, "%s/%s", path1, folder);

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
                char noname[1000];

                snprintf(noname, noext, "%s", name);
                enkrip(noname);

                strcat(noname, ext);
                sprintf(path2, "%s/%s", path, noname);
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

//fungsi logsystem
void logging(int level, char *comm, char *first, char *last){
    FILE *f1 = fopen(logsys, "a+");
    int year, month, day, hour, minute, second;
    char rev[1000], lvl[25];
    memset(rev, 0, 1000*sizeof(char));
    memset(lvl, 0, 25*sizeof(char));

    time_t t;
    struct tm* loc;
    time(&t);
    loc = localtime(&t);

    year = (loc->tm_year + 1900)%1000;
    month = loc->tm_mon+1;
    day = loc->tm_mday;
    hour = loc->tm_hour;
    minute = loc->tm_min;
    second = loc->tm_sec;

    if(lvl == 0){
        strcpy(lvl, "WARNING");
    }
    else{
        strcpy(lvl,"INFO");
    }

    if(last == NULL){
        sprintf(rev,"%s::%d%d%d-%d:%d:%d::%s::%s\n", lvl, year, month, day, hour, minute, second, comm, first);
    }
    else{
        sprintf(rev,"%s::%d%d%d-%d:%d:%d::%s::%s::%s\n", lvl, year, month, day, hour, minute, second, comm, first, last);
    }

    fclose(f1);
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

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){

    char fpath[1000], name[1000];

    if (strcmp(path, "/") == 0){
		sprintf(fpath, "%s", dirpath);
	} else {
		sprintf(name,"%s", path);
		sprintf(fpath, "%s%s", dirpath, name);
	}

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

//untuk membuat direktori
static int xmp_mkdir(const char *path, mode_t mode){
	int res;

	char fpath[1000];

    sprintf(fpath,"%s%s", dirpath, path);

    logging(1, "MKDIR", fpath, NULL);

	res = mkdir(fpath, mode);
	if (res == -1){
		return -errno;
    }

	return 0;
}

static int xmp_rename(const char *from, const char *to){
	int res;
    char fpath[1000], fpath1[1000], name[1000];

    sprintf(fpath, "%s%s", dirpath, from);
    sprintf(fpath1, "%s%s", dirpath, to);

    if(strncmp(to, enc1, 7) == 0){
        sprintf(name, "%s", to);
        enk(fpath);
    }

    logging(1, "RENAME", fpath, fpath1);

	res = rename(fpath, fpath1);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path){
    int res;

    char fpath[1000];

    sprintf(fpath,"%s%s",dirpath,path);

    logging(0,"RMDIR",fpath,NULL);

    res = rmdir(fpath);
    if(res==-1){
        return -errno;
    }
    return 0;
}


static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    char fpath[1000];
    if(strcmp(path,"/") == 0){
        path = dirpath;
        sprintf(fpath,"%s", path);
    }
    else {
        sprintf(fpath, "%s%s", dirpath, path);
    }
    int res = 0, fd = 0 ;
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1){
        return -errno;    
    }
    
    res = pread(fd, buf, size, offset);
    if (res == -1){
        res = -errno;
    }
    
    close(fd);
    return res;
}

//untuk membuat file
static int xmp_mknod(const char *path, mode_t mode, dev_t rdev){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s", dirpath, path);
    logging(1, "MKNODE", fpath, NULL);
    res = mknod(fpath, mode, rdev);
    if(res == -1){
        return -errno;
    }

    return 0;
}

//untuk menghapus file
static int xmp_unlink(const char *path){
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);
    logging(0, "UNLINK", fpath, NULL);
    res = unlink(fpath);
    if(res == -1){
        return -errno;
    }

    return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    char fpath[1000], name[1000];
    int fd, res;

    sprintf(name,"%s",path);

    if(strcmp(path,"/") == 0){
        path = dirpath;
        sprintf(fpath,"%s", path);
    }
    else{
        sprintf(fpath, "%s%s", dirpath, name);
    }

    (void) fi;
    fd = open(fpath, O_WRONLY);
    res = pwrite(fd, buf, size, offset);

    if (fd == -1){
        return -errno;
    }
    
    if (res == -1){
        res = -errno;
    }

    close(fd);
    return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.rename		= xmp_rename,
    .rmdir      = xmp_rmdir,
    .read       = xmp_read,
    .mknod      = xmp_mknod,
    .unlink     = xmp_unlink,
    .write      = xmp_write,
};

int main(int argc, char *argv[]){
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
