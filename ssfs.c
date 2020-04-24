#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
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

void enkrip(char *src){
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
        sprintf(path, "%s%s", src, name);

        if(strcmp(name, ".") && strcmp(name, "..") && d->d_type == DT_DIR){
            char folder[1000];

            strcpy(folder, name);
            enkrip(folder);

            strcpy(path1, path);
            sprintf(path2, "%s%s", path1, folder);

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
                char noname[100];

                snprintf(noname, noext, "%s", name);
                enkrip(noname);

                sprintf(name, "%s%s", noname, ext);
                sprintf(path2, "%s/%s", path, name);
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

void dekrip(char *src){
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

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

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

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}
    

	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}


static int xmp_rename(const char *from, const char *to)
{
	int res;
    char fpath[1000];
    char tpath[1000];

    sprintf(fpath, "%s%s", dirpath, from);
    sprintf(tpath, "%s%s", dirpath, to);

    if(!strncmp(enc1, to, 7)){
        enk((char*)fpath);
    }

	res = rename(fpath, tpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    int res;
    char fpath[1000];

    if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.truncate	= xmp_truncate,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.create     = xmp_create,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}


