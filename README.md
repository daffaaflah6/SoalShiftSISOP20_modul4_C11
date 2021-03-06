# SoalShiftSISOP20_modul4_C11

Marsha Nabilah Wibowo - 05111840000027 || Muhammmad Daffa' Aflah Syarif - 05111840000030

Source Code : [ssfc.c](https://github.com/daffaaflah6/SoalShiftSISOP20_modul4_C11/blob/master/ssfs.c)

# 1. Enkripsi versi 1
- Jika sebuah direktori dibuat dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
- Jika sebuah direktori di-rename dengan awalan “encv1_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v1.
- Apabila sebuah direktori terenkripsi di-rename menjadi tidak terenkripsi, maka isi adirektori tersebut akan terdekrip.
- Setiap pembuatan direktori terenkripsi baru (mkdir ataupun rename) akan tercatat ke sebuah database/log berupa file.
- Semua file yang berada dalam direktori ter enkripsi menggunakan caesar cipher dengan key.

```9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO```

Misal kan ada file bernama “kelincilucu.jpg” dalam directory FOTO_PENTING, dan key yang dipakai adalah 10
“encv1_rahasia/FOTO_PENTING/kelincilucu.jpg” => “encv1_rahasia/ULlL@u]AlZA(/g7D.|_.Da_a.jpg
Note : Dalam penamaan file ‘/’ diabaikan, dan ekstensi tidak perlu di encrypt.
- Metode enkripsi pada suatu direktori juga berlaku kedalam direktori lainnya yang ada didalamnya.

```c
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
```
- Fungsi di atas digunakan untuk mengetahui index dari file yang akan di-encrypt dan di-decrypt tanpa mengikutsertakan slash dan ekstensi dari path awal.

```c
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
```
- Fungsi enk akan dipanggil di ```xmp_readdir``` agar FUSE melakukan encrypt di FUSE.

```c
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
```
- Fungsi dek dipanggil di semua fungsi yang lain seperti ```mkdir```, ```read```, ```write```, dll. Fungsi dek dipanggil agar pada saat FUSE dijalankan, decrypt dilakukan di mount folder.

```c
char *a = strstr(path, enc1);
	if(a != NULL)
		dek(a);
```
- Pada saat pemanggilan dek, akan ada pemeriksaan menggunakan ```strstr``` agar isi folder yang di-decrypt hanya yang berawalan encv1_.

# 2. Enkripsi versi 2
- Jika sebuah direktori dibuat dengan awalan “encv2_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v2.
- Jika sebuah direktori di-rename dengan awalan “encv2_”, maka direktori tersebut akan menjadi direktori terenkripsi menggunakan metode enkripsi v2.
- Apabila sebuah direktori terenkripsi di-rename menjadi tidak terenkripsi, maka isi direktori tersebut akan terdekrip.
- Setiap pembuatan direktori terenkripsi baru (mkdir ataupun rename) akan tercatat ke sebuah database/log berupa file.
- Pada enkripsi v2, file-file pada direktori asli akan menjadi bagian-bagian kecil sebesar 1024 bytes dan menjadi normal ketika diakses melalui filesystem rancangan jasir. Sebagai contoh,
```
file File_Contoh.txt berukuran 5 kB pada direktori asli akan menjadi 5 file kecil yakni: File_Contoh.txt.000, File_Contoh.txt.001, File_Contoh.txt.002, File_Contoh.txt.003, dan File_Contoh.txt.004.
```
- Metode enkripsi pada suatu direktori juga berlaku kedalam direktori lain yang ada didalam direktori tersebut (rekursif).

- Fungsi enkripsi 2:
```c
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
						char *argv[]={"rm", awal1, NULL};
						execv("/bin/rm", argv);
					}	
				}
			}
		}
    }
	return;
}
```
- Fungsi ini akan dipanggil pada ```xmp_rename```
- Fungsi menggunakan fork, wait, dan exec untuk memecah file menjadi 1kb.
- Dilakukan rekursi apabila yang ditemui adalah directory.
- Menggunakan split dan remove untuk memecah file dan menghapus setelah file sudah selesai dipecah.

```c
if (strstr(to, enc2) != NULL){
		enk2(fpath1);
		logging(1, "ENCRYPT2", fpath, fpath1);
	}
```
- Pada ```xmp_rename``` dilakukan pengecekan strstr terhadap path yang akan di-rename. Sehingga apabila path yang di-rename menjadi encv2_, isi dari folder tersebut akan dipecah.

# 3. Sinkronisasi Direktori Otomatis
Tanpa mengurangi keumuman, misalkan suatu directory bernama dir akan tersinkronisasi dengan directory yang memiliki nama yang sama dengan awalan sync_ yaitu sync_dir. Persyaratan untuk sinkronisasi yaitu:
- Kedua directory memiliki parent directory yang sama.
- Kedua directory kosong atau memiliki isi yang sama. Dua directory dapat dikatakan memiliki isi yang sama jika memenuhi:
```
Nama dari setiap berkas di dalamnya sama.
Modified time dari setiap berkas di dalamnya tidak berselisih lebih dari 0.1 detik.
```
- Sinkronisasi dilakukan ke seluruh isi dari kedua directory tersebut, tidak hanya di satu child directory saja.
- Sinkronisasi mencakup pembuatan berkas/directory, penghapusan berkas/directory, dan pengubahan berkas/directory.
Jika persyaratan di atas terlanggar, maka kedua directory tersebut tidak akan tersinkronisasi lagi.
Implementasi dilarang menggunakan symbolic links dan thread.

# 4. Log System
- Sebuah berkas nantinya akan terbentuk bernama "fs.log" di direktori *home* pengguna (/home/[user]/fs.log) yang berguna menyimpan daftar perintah system call yang telah dijalankan.
- Agar nantinya pencatatan lebih rapi dan terstruktur, log akan dibagi menjadi beberapa level yaitu INFO dan WARNING.
- Untuk log level WARNING, merupakan pencatatan log untuk syscall rmdir dan unlink.
- Sisanya, akan dicatat dengan level INFO.
- Format untuk logging yaitu:

```
[LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC ...]
```

LEVEL     : Level logging
yy        : Tahun dua digit
mm        : Bulan dua digit
dd        : Hari dua digit
HH        : Jam dua digit
MM        : Menit dua digit
SS        : Detik dua digit
CMD       : System call yang terpanggil
DESC      : Deskripsi tambahan (bisa lebih dari satu, dipisahkan dengan ::)

Contoh format logging nantinya seperti:

```
INFO::200419-18:29:28::MKDIR::/iz1
INFO::200419-18:29:33::CREAT::/iz1/yena.jpg
INFO::200419-18:29:33::RENAME::/iz1/yena.jpg::/iz1/yena.jpeg
```

Untuk fungsi log systemnya seperti berikut
```
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
```

Kemudian fungsi log system akan dipanggil di beberapa fungsi dibawah ini untuk  FUSE menjalankan fungsi log systemnya :
- xmp_mkdir dengan cara `logging(1, "MKDIR", fpath, NULL);`
- xmp_rmdir dengan cara `logging(0,"RMDIR",fpath,NULL);`
- xmp_rename dengan cara `logging(1, "RENAME", fpath, fpath1);`
- xmp_mknod dengan cara `logging(1, "MKNODE", fpath, NULL);`
- xmp_unlink dengan cara `logging(0, "UNLINK", fpath, NULL);`
