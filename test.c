#define RAM_DIR "/usr/myramtest"
#define DSK_DIR "/usr/mydisktest"

#define KB 1024
#define MB 1048576
#define ALIGNSZ 512
#define ALIGN(X) (((X + ALIGNSZ - 1) / ALIGNSZ) * ALIGNSZ)

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* blocksize for different tests */
const int blksize[] = {64, 256, 1 * KB, 4 * KB, 16 * KB, 64 * KB};

/* number of procs in different tests */
const int numproc[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

/* each test repeat rep times */
const int rep = 5;

/* each test proc run secs time before termination */
const int secs = 7;

/* whether the alarm signal arrived */
volatile int alarm_sig_arrived = 0;
void alarm_handler(int sig) { alarm_sig_arrived = 1; }

/* default file size */
const int tot_fsize = 400 * MB;

/* make a file with given size */
int create_big(const int fsize, const char fpath[]) {
    printf("create & fill file [%s]\n", fpath);
    char buff[128 * KB];
    for (int i = 0; i < 128 * KB; i++)
        buff[i] = rand() % 10 + '0';
    int fd = open(fpath, O_CREAT | O_TRUNC | O_WRONLY);
    int step = (fsize / (128 * KB) + 1) / 64;
    for (int i = 0; i < fsize / (128 * KB) + 1; i++)
        write(fd, buff, 128 * KB);
    close(fd);
    return 0;
}

void do_test(
    int test_id,   /* test id */
    int proc_id,   /* process id */
    int isdisk,    /* whether this file is on disk */
    int iswrite,   /* read test or write test */
    int isordered, /* ordered test or random test */
    int bsize,     /* block size for each operation */
    int fsize,     /* file size for this test (adds up to tot_fsize) */
    int proc_n     /* number of processes */
) {
    printf("test [%04d] proc [%04d] start\n", test_id, proc_id);
    /* we set alarm_sig_arrived to 0 initially */
    alarm_sig_arrived = 0;

    /* buffer, file name */
    char buf[ALIGN(bsize)], fpath[128];
    for (int i = 0; i < ALIGN(bsize); i++)
        buf[i] = rand() % 10 + '0';

    /* if is disk, choose disk dir, else choose ram dir */
    char* dir = isdisk ? DSK_DIR : RAM_DIR;
    sprintf(fpath, "%s/%03d%05d%01d%01d%01d%05d",
            dir, test_id, proc_id, isdisk, iswrite, isordered, bsize);

    /* open file at fpath, mind that we use O_DIRECT flag to keep all data flushed to the mem hardware */
    int flag = O_DIRECT | (iswrite ? O_WRONLY : O_RDONLY);
    int fd = open(fpath, flag);

    /* byte count, initially 0 */
    long long byte_cnt = 0;

    /* read/write cnt, initially 0 */
    long long io_cnt = 0;

    /* set signal handler */
    printf("test [%04d] proc [%04d] set sig alarm\n", test_id, proc_id);
    signal(SIGALRM, alarm_handler);
    /* set alarm secs later */
    alarm(secs);

    /* run until alarm rings */
    while (!alarm_sig_arrived) {
        long long oksize = 0;
        printf("%d\n", ALIGN(bsize));
        if (!isordered)
            lseek(fd, ALIGN((rand() % fsize) - bsize), 0);
        if (iswrite) {
            oksize = write(fd, buf, bsize);
        } else {
            oksize = read(fd, buf, bsize);
        }
        if (~oksize) {
            byte_cnt += oksize, ++io_cnt;
            if (oksize == 0) lseek(fd, 0, 0);
        } else {
            printf("test [%04d] proc [%04d] fail\n", test_id, proc_id);
        }
    }

    /* print message when alarm rings */
    printf(
        "test [%04d] proc [%04d] alarm rings\n"
        "byte cnt [%04lld]\n"
        "io cnt [%04lld]\n",
        test_id, proc_id, byte_cnt, io_cnt);

    /* calculate throughput, print data */
    double throughput = (double)(byte_cnt) / (double)(secs);
    double latency = (double)(secs) / (double)(io_cnt);

    /* print data to row */
    char row[256];
    memset(row, 0, sizeof(row));
    sprintf(row, "%d, %d, %d, %d, %d, %d, %.4lf, %.10lf, %d\n",
            test_id, proc_id, isdisk, iswrite, isordered, bsize,
            throughput, latency, proc_n);

    /* open serial device, flush one data row */
    printf("test [%04d] proc [%04d] printf data to serial\n",
           test_id, proc_id);
    int data_fd = open("/dev/tty00", O_WRONLY);
    write(data_fd, row, strlen(row));
    close(data_fd);

    /* exit this proc */
    exit(0);
}

int test_one_rep(
    const int proc_num,  /* number of processes */
    const int isdisk,    /* whether this file is on disk */
    const int iswrite,   /* read test or write test */
    const int isordered, /* ordered test or random test */
    const int bsize      /* block size for each operation */
) {
    /* file size attached to each test */
    const int fsize = tot_fsize / proc_num + 1;
    for (int r = 0; r < rep; r++) {
        printf("\n\n");
        printf("test [%-2d] procs [%-2d]\n", r, proc_num);
        printf(
            "device [%s]"
            " [%s] [%s] [%.5f KB]\n",
            isdisk ? "disk" : "ram",
            iswrite ? "write" : "read",
            isordered ? "ordered" : "random",
            (double)bsize / (double)KB);
        printf("--------------------------------------------------\n");
        int cpid[proc_num];
        for (int i = 0; i < proc_num; i++) {
            /* if is disk, choose disk dir, else choose ram dir */
            char fpath[128];
            const char* dir = isdisk ? DSK_DIR : RAM_DIR;
            sprintf(fpath, "%s/%03d%05d%01d%01d%01d%05d",
                    dir, r, i + 1, isdisk, iswrite, isordered, bsize);
            /* make file with given size (because we want to do some lseek tests) */
            create_big(fsize, fpath);
        }
        for (int i = 0; i < proc_num; i++) {
            int pid = fork();
            if (pid != 0) {
                cpid[i] = pid;
            } else {
                do_test(r, i + 1, isdisk, iswrite, isordered, ALIGN(bsize), fsize, proc_num);
                return 0;
            }
        }
        for (int i = 0; i < proc_num; i++)
            waitpid(cpid[i], NULL, 0);
        printf("all tests finished\n");
        printf("--------------------------------------------------\n");
        printf("remove test files\n");
        for (int i = 0; i < proc_num; i++) {
            /* if is disk, choose disk dir, else choose ram dir */
            char fpath[128];
            const char* dir = isdisk ? DSK_DIR : RAM_DIR;
            sprintf(fpath, "%s/%03d%05d%01d%01d%01d%05d",
                    dir, r, i + 1, isdisk, iswrite, isordered, bsize);
            /* remove files (leave space for further use) */
            printf("remove [%s]\n", fpath);
            remove(fpath);
        }
        printf("remove file finished\n");
        printf("--------------------------------------------------\n");
    }
    return 0;
}

int main() {
    /* set random seed */
    srand((unsigned)time(NULL));
    /* add heading */
    const int data_fd = open("/dev/tty00", O_WRONLY);
    const char* heading =
        "repeat_id, "
        "pid, "
        "ram_or_disk, "
        "read_or_write, "
        "ordered_or_random, "
        "block_size, "
        "throughput(per_second), "
        "latency(second_per_io), "
        "proc_n\n";
    write(data_fd, heading, strlen(heading));
    close(data_fd);
    /* run all the tests */
    for (int iswrite = 0; iswrite < 2; iswrite++) {
        for (int i = 0; i < sizeof(blksize) / sizeof(int); i++) {
            for (int j = 0; j < sizeof(numproc) / sizeof(int); j++) {
                for (int isordered = 0; isordered < 2; isordered++) {
                    for (int isdisk = 0; isdisk < 2; isdisk++) {
                        test_one_rep(
                            numproc[j],
                            isdisk,
                            iswrite,
                            isordered,
                            blksize[i]);
                    }
                }
            }
        }
    }
    return 0;
}
