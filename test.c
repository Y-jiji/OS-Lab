#define RAM_DIR "/root/myramtest"
#define DSK_DIR "/root/mydisktest"

#define KB 1024
#define MB 1048576

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
const int rep = 128;
/* each test proc run secs time before termination */
const int secs = 15;

/* whether the alarm signal arrived */
volatile int alarm_sig_arrived = 0;
void alarm_handler(int sig) { alarm_sig_arrived = 1; }

/* default file size */
const int tot_fsize = 400 * MB;

/* make a file with given size */
int make_big(int fsize, char* fpath) {
    printf("making file [%s]\n", fpath);
    char buff[1024];
    for (int i = 0; i < 1024; i++)
        buff[i] = rand() % 10 + '0';
    int fd = open(fpath, O_CREAT | O_TRUNC | O_WRONLY);
    for (int i = 0; i < fsize / 1024 + 1; i++)
        write(fd, buff, 1024);
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
    int fsize      /* file size for this test (adds up to tot_fsize) */
) {
    /* we set alarm_sig_arrived to 0 initially */
    alarm_sig_arrived = 0;

    /* buffer, file name */
    char buf[bsize + 16], fpath[128];
    for (int i = 0; i < bsize + 16; i++)
        buf[i] = rand() % 10 + '0';

    /* if is disk, choose disk dir, else choose ram dir */
    char* dir = isdisk ? DSK_DIR : RAM_DIR;
    sprintf(fpath, "%s/%03d%05d%01d%01d%01d%01d",
            dir, test_id, proc_id, isdisk, iswrite, isordered, bsize);

    /* open file at fpath, mind that we use O_SYNC flag to keep all data flushed to the mem hardware */
    int flag = O_SYNC | (iswrite ? O_WRONLY : O_RDONLY);
    int fd = open(fpath, flag);

    /* byte count, initially 0 */
    long long byte_cnt = 0;

    /* set signal handler */
    signal(SIGALRM, alarm_handler);
    /* set alarm secs later */
    alarm(secs);

    /* run until alarm rings */
    while (!alarm_sig_arrived) {
        long long oksize = 0;
        if (!isordered)
            lseek(fd, (rand() % fsize) - bsize, 0);
        if (iswrite) {
            oksize = write(fd, buf, bsize);
        } else {
            oksize = read(fd, buf, bsize);
        }
        if (~oksize) byte_cnt += oksize;
    }

    /* calculate throughput, print data */
    double throughput = (double)(byte_cnt) / (double)(secs);

    /* print data to row */
    char row[1024];
    sprintf(row, "%d, %d, %d, %d, %d, %d, %.4lf\r\n",
            test_id, proc_id, isdisk, iswrite, isordered, bsize, throughput);

    /* open serial device, flush one data row */
    int data_fd = open("/dev/tty00", O_WRONLY);
    write(data_fd, row, 1024);
    close(data_fd);

    /* exit this proc */
    exit(0);
}

int test_one_rep(
    int proc_num,  /* number of processes */
    int isdisk,    /* whether this file is on disk */
    int iswrite,   /* read test or write test */
    int isordered, /* ordered test or random test */
    int bsize      /* block size for each operation */
) {
    /* file size attached to each test */
    int fsize = tot_fsize / proc_num + 1;
    for (int r = 0; r < rep; r++) {
        int cpid[proc_num];
        for (int i = 0; i < proc_num; i++) {
            /* if is disk, choose disk dir, else choose ram dir */
            char fpath[128];
            char* dir = isdisk ? DSK_DIR : RAM_DIR;
            sprintf(fpath, "%s/%03d%05d%01d%01d%01d%01d", 
                    dir, r, i+1, isdisk, iswrite, isordered, bsize);
            /* make file with given size (because we want to do some lseek tests) */
            make_big(fsize, fpath);
        }
        for (int i = 0; i < proc_num; i++) {
            int pid = fork();
            if (pid) {
                cpid[i] = pid;
                continue;
            }
            do_test(r, i + 1, isdisk, iswrite, isordered, bsize, fsize);
        }
        for (int i = 0; i < proc_num; i++)
            waitpid(cpid[i], NULL, 0);
    }
    return 0;
}

int main() {
    /* random seed */
    srand((unsigned)time(NULL));
    /* run all the tests */
    for (int isdisk = 0; isdisk < 2; isdisk++) {
        for (int iswrite = 0; iswrite < 2; iswrite++) {
            for (int isordered = 0; isordered < 2; isordered++) {
                for (int i = 0; i < sizeof(blksize); i++) {
                    for (int j = 0; j < sizeof(numproc); j++) {
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
