#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE "/dev/DinhAnhDung"
#define MAX 1000

void decode(char *buff, int cea)
{
    int i;
    int n = strlen(buff);
    for(i = 0; i< n; i++)
    {
        buff[i] -= cea;
    }
}

int main(){
    int i, fd, ch;
    char * write_buf;
    char * read_buf;

    fd = open(DEVICE, O_RDWR); // open for reading and writing

    if(fd == -1)
    {
        printf("file %s either does not exist or has been locked by another process\n", DEVICE);
        exit(-1);
    }

    while(ch != 5)
    {
        read_buf = (char *)malloc(sizeof(char) * MAX);
        write_buf = (char *)malloc(sizeof(char) * MAX);
        printf("Menu\n");
        printf("1. Read from Device\n");
        printf("2. Write to device\n");
        printf("3. Clear device\n");
        printf("4. Decode\n");
        printf("5. Exit\n");
        printf("Your choice: ");
        scanf("%d", &ch);
        while(getchar() != '\n');

        if (ch == 1)
        {
            read(fd, read_buf, sizeof(read_buf));
            printf("secret information: \n%s\n", read_buf);
        }

        if(ch == 2)
        {
            read(fd, read_buf, sizeof(read_buf));
            char* temp = (char *)malloc(sizeof(char) * MAX);
            strcpy(temp, read_buf);
            strcat(temp, "\n");
            gets(write_buf);
            strcat(temp, write_buf);
            write(fd, temp, sizeof(temp));
            // gets(write_buf);
            // write(fd, write_buf, sizeof(write_buf));
            printf("Writed to device\n");
        }

        if(ch == 3)
        {
            strcpy(write_buf, "");
            write(fd, write_buf, sizeof(write_buf));
            printf("All messages in devices clears\n");
        }

        if(ch == 4)
        {
            printf("input the ceasar: ");
            int cea;
            scanf("%d", &cea);
            while(getchar() != '\n');
            read(fd, read_buf, sizeof(read_buf));
            int n = strlen(read_buf);
            int i;
            for(i = 0; i< n; i++)
            {
                read_buf[i] = read_buf[i] - cea;
                printf("%c", read_buf[i]);
            }
            printf("\n");
        }
    }


    // printf("r = read from device\n w = write to device\n enter command: ");
    // scanf("%c", &ch);
    // while(getchar() != '\n');


    // switch(ch)
    // {
    //     case 'w':
    //     printf("enter data: ");
    
    //     gets(write_buf);
    //     write(fd, write_buf, sizeof(write_buf));
    //     break;
        
    //     case 'r':
    //     read(fd, read_buf, sizeof(read_buf));
    //     printf("device: %s\n", read_buf);
    //     break;

    //     default:
    //     printf("command not recognized\n");
    //     break;
    // }
    // return 0;
}