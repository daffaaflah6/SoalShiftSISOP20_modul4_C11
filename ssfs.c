#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

static const char *dirpath = "/home/bela/shift4";

char *list = {"9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO"};
const int k = 10;
char enc1[10] = "encv1_";
char enc2[10] = "encv2_";
static const char *logsys = "/home/bela/fs.log";
char temp_sync[1000];

int ext(char* path){
	int i;
	int len = strlen(path);

	for(i=len-1; i >=0; i--){
		if(path[i] == '.'){
			return i;
		}
	}

	return len;
}

int slash(char* path, int bil){
	int i;
	int len = strlen(path);

	for(i=0; i<len; i++){
		if(path[i] == '/'){
			return i + 1;
		}
	}

	return bil;
}

void enk(char *path)		// ke kiri
{
	if (!strcmp(path, ".") || !strcmp(path, ".."))
		return;

	int temp = 0, i, j;

	int end = ext(path);
	int start = slash(path, 0);

	for (i = start; i < end; i++)
	{
		if(path[i] != '/'){
			for (j = 0; j < strlen(list); j++)
			{
				if (path[i] == list[j])
				{
					temp = (j + k) % strlen(list);
					path[i] = list[temp];
					break;
				}
			}
		}
	}
}

void dek(char *path)
{
	if(!strcmp(path, ".") || !strcmp(path, ".."))
		return;

	int temp = 0, i, j;

	int end = ext(path);
	int start = slash(path, end);

	for (i = start; i < end; i++)
	{
		if(path[i] != '/'){
			for (j = 0; j < strlen(list); j++)
			{
				if (path[i] == list[j])
				{
					temp = (j + (strlen(list) - k)) % strlen(list);
					path[i] = list[temp];
					break;
				}
			}
		}
	}
}

void enk2(char *path)
{
	chdir(path);
	DIR *d;
    struct dirent *dp;
	struct stat st;
    d = opendir(".");

    if (d){
        while ((dp = readdir(d)) != NULL){
			if (stat(dp->d_name, &st) < 0);
			else if (S_ISDIR(st.st_mode)){
				if (strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0) continue;
				char fpath[1000];
				sprintf(fpath,"%s/%s", path, dp->d_name);
				enk2(fpath);
			}else{
				char awal1[1000];
				sprintf(awal1,"%s/%s", path, dp->d_name);
				char awal2[3000];
				sprintf(awal2,"%s.", awal1);
				pid_t child_id = fork();
				int status;
				if (child_id == 0){
					pid_t child = fork();
					if (child == 0){
						execl("/usr/bin/split", "split","-b","1024","-a","3","-d", awal1, awal2, NULL);
					}else {
						while ((wait(&status))>0);
						execl("/bin/rm", awal1, NULL);
					}	
				}
			}
		}
    }
	return;
}

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

    if(level == 0){
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

char* add_sync(char *path){
    memset(temp_sync,0,100*sizeof(char));
    sprintf(temp_sync,"sync_%s",path);
    return temp_sync;
}

void del_sync(char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char fpath[1000];

	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					   off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];

	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
        del_sync(fpath,"sync_");
	}

	int res = 0;
	DIR *dp;
	struct dirent *de;

	(void)offset;
	(void)fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		printf("before: %s\n", de->d_name);
		if(a != NULL)
			enk(de->d_name);
		printf("after: %s\n", de->d_name);

		res = (filler(buf, de->d_name, &st, 0));

		if (res != 0)
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
					struct fuse_file_info *fi)
{
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
        del_sync(fpath,"sync_");
	}

	int res = 0;
	int fd = 0;

	(void)fi;

	fd = open(fpath, O_RDONLY);

	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
	char fpath[1000];

	char *a = strstr(path, enc1);

	if(a != NULL){
		dek(a);
	}

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
	logging(1, "MKDIR", fpath, NULL);

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
	logging(0,"RMDIR",fpath,NULL);

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
	char fpath[1000], fpath1[1000];

	char *a = strstr(to, enc1);

	if(a != NULL)
		dek(a);

	sprintf(fpath, "%s%s", dirpath, from);
	sprintf(fpath1, "%s%s", dirpath, to);

    del_sync(fpath,"sync_");
    del_sync(fpath1,"sync_");
	

	res = rename(fpath, fpath1);
	if (res == -1)
		return -errno;

	logging(1, "RENAME", fpath, fpath1);

	if (strstr(to, enc2) != NULL){
		enk2(fpath1);
		logging(1, "ENCRYPT2", fpath, fpath1);
	}

	// if (strstr(from, enc2) != NULL && strstr(to, enc2) == NULL){
	// 	dek2(fpath1);
	// 	logging(1, "DECRYPT2", fpath, fpath1);
	// }

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
					 off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	(void)fi;

	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);

	return res;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	(void)fi;

	int res;
	char fpath[1000];

	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = creat(fpath, mode);
	if (res == -1)
		return -errno;

	close(res);

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	char fpath[1000];

	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	char fpath[1000];

	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) {
	int res;
	char fpath[1000];
	char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	}else{
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
	logging(1, "MKNODE", fpath, NULL);

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
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

static int xmp_unlink(const char *path){
    int res;
    char fpath[1000];
    char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);

	if(strcmp(path, "/") == 0){
		path = dirpath;
		sprintf(fpath, "%s", path);
	} else {
		sprintf(fpath, "%s%s", dirpath, path);
	}

    del_sync(fpath,"sync_");
    logging(0, "UNLINK", fpath, NULL);
    res = unlink(fpath);
    if(res == -1){
        return -errno;
    }

    return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr = xmp_getattr,
	.readdir = xmp_readdir,
	.read = xmp_read,
	.mkdir = xmp_mkdir,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	.truncate = xmp_truncate,
	.write = xmp_write,
	.create = xmp_create,
	.utimens = xmp_utimens,
	.access = xmp_access,
	.open = xmp_open,
	.mknod = xmp_mknod,
	.unlink = xmp_unlink,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
