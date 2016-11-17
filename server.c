/* Name: Tushev, Miroslav
 * Project: PA-3a
 * Instructor: Feng Chen
 * Class: cs7103-au16
 * LogonID: cs710305
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* size of buffer used for message exchange */
#define BUFSIZE 1000

/* colors for terminal */
#define SYS "\x1B[32m"
#define USR "\x1B[35m"
#define RES "\x1B[0m"

/* public board stuff */
char postings[10][BUFSIZE];        // 10 postings
int cur_posting = 0;               // current posting

/* for threads */
struct args_st {
  char ipAddress[INET_ADDRSTRLEN]; // ip address of a client
  int newsockfd;                   // socket descriptor
};
pthread_mutex_t users_lock;
pthread_mutex_t posting_lock;

/* for storing each user's info */
struct cred {
  char name[20];
  char ip[INET_ADDRSTRLEN];
  char timebuf[9];                  // time when a user connected
};

struct cred users[100];             // 100 users max
int cur_user = 0;                   // used for adding users

/* for extracting user name */
void name_copy (char *src, char *dest) {
  for (int i = 0; i != strlen(src); i++) {
    if (src[i] == '\n') {
      dest[i] = '\0';
      break;
    }
    else
      dest[i] = src[i];
  }
}

/* for getting system's time */
void get_time(char *tme) {
  time_t timer;
  char timebuf[9];
  struct tm* tm_info;
  time(&timer);
  tm_info = localtime(&timer);
  strftime(timebuf, 9, "%H:%M:%S", tm_info);
  strcpy(tme, timebuf);
}

/* main function for clients */
void *client_processing (void* args) {
    // unpack the arguments
    struct args_st *arguments = (struct args_st *) args;
    int sock = arguments->newsockfd;
    char *ipAddress = arguments->ipAddress;

    int n = 0;
    char buffer[BUFSIZE];
    memset(buffer,'\0',BUFSIZE);
    FILE *log;
    log = fopen("log.txt", "a+");

    // time related stuff
    char timebuf[9];

    // login first
    n = read(sock,buffer,BUFSIZE);
    char user[20];
    int usr_id = 0;
    name_copy(buffer, user);
    // add user to database
    get_time(timebuf);
    pthread_mutex_lock(&users_lock);
    strcpy(users[cur_user].name, user);
    strcpy(users[cur_user].ip, ipAddress);
    strcpy(users[cur_user].timebuf, timebuf);
    usr_id = cur_user;
    cur_user++;
    pthread_mutex_unlock(&users_lock);

    memset(buffer,'\0',BUFSIZE);

    if (strncmp("admin", user, 5) == 0) {
      strcpy(buffer, "Please enter the password");
      n = write(sock, buffer, BUFSIZE);
      memset(buffer,'\0',BUFSIZE);
      n = read(sock, buffer, BUFSIZE);
      if (strncmp("password", buffer, 8) != 0) {
        strcpy(buffer, "Sorry, the password is incorrect. Disconnecting");
        n = write(sock, buffer, BUFSIZE);
        memset(users[usr_id].name, '\0', 20);
        get_time(timebuf);
        printf("%s%s System%s: %s disconnected\n", SYS, timebuf, RES, user);
        fprintf(log, "%s System: %s disconnected\n", timebuf, user);
        fflush(log);
        return 1;
      }
      else {
        get_time(timebuf);
        printf("%s%s System%s: admin connected\n", SYS, timebuf, RES);
        fprintf(log, "%s System: admin connected\n", timebuf);
        fflush(log);
        memset(buffer,'\0',BUFSIZE);
        strcpy(buffer, "Hello, admin");
        n = write(sock, buffer, BUFSIZE);
        memset(buffer,'\0',BUFSIZE);
      }
    }
    else {
      get_time(timebuf);
        printf("%s%s System%s: %s connected\n", SYS, timebuf, RES, user);
        fprintf(log, "%s System: %s connected\n", timebuf, user);
        fflush(log);
        strcpy(buffer, "Hello, ");
        strcat(buffer, user);
        strcat(buffer, "\n");
        strcat(buffer, "You are connected! The following users are online:\n");
        strcat(buffer, USR);
        for (int i = 0; i <= cur_user; i++) {
          strcat(buffer, users[i].name);
          strcat(buffer, " ");
        }
        strcat(buffer, RES);
        strcat(buffer, "\nPost your first message using\n"
                       "\"write\", or read the messages posted by other\n"
                       "users using \"read\"! For help type \"help\"");
        n = write(sock, buffer, BUFSIZE);
        memset(buffer,'\0',BUFSIZE);
    }


    while((n = read(sock,buffer,BUFSIZE))) {

       // = read(sock,buffer,255);

       if (n < 0) {
         get_time(timebuf);
         fprintf(log, "%s System: ERROR on reading from socket\n", timebuf);
         fflush(log);
         return 1;
       }

      /* commands go here */
      if (strncmp("help", buffer, 4) == 0) {
        memset(buffer,'\0',BUFSIZE);
        strcpy(buffer,"*******************************************\n"
                         "help          - print the commands available\n"
                         "write MESSAGE - send a MESSAGE to the board\n"
                         "read          - read the messages posted\n"
                         "*******************************************");
        n = write(sock, buffer, BUFSIZE);
        memset(buffer,'\0',BUFSIZE);
      }
      // write command
      else if (strncmp("write", buffer, 5) == 0) {
      get_time(timebuf);
            printf("%s%s System%s: write command requested\n", SYS, timebuf, RES);
            fprintf(log, "%s System: write command requested\n", timebuf);
            fflush(log);
            pthread_mutex_lock(&posting_lock);
            strcpy(postings[cur_posting], USR);
            strcat(postings[cur_posting], timebuf);
            fprintf(log, timebuf);
            strcat(postings[cur_posting], " ");
            fprintf(log, " ");
            strcat(postings[cur_posting], user);
            fprintf(log, user);
            strcat(postings[cur_posting], RES);
            strcat(postings[cur_posting], ": ");
            fprintf(log, ": ");
            strncat(postings[cur_posting], buffer+6, BUFSIZE-6);
            printf("%s", postings[cur_posting]);
            fprintf(log, buffer+6);
            fprintf(log, "\n");
            fflush(log);
            if (cur_posting == 9)
                cur_posting = 0;
            else
                cur_posting++;
            pthread_mutex_unlock(&posting_lock);
            memset(buffer,'\0',BUFSIZE);
            strcpy(buffer, "The message has been succesfully posted!");
            n = write(sock, buffer, BUFSIZE);
             if (n < 0) {
              perror("ERROR writing to socket");
              return 1;
           }
            memset(buffer,'\0',BUFSIZE);
       }
       // read command
       else if (strncmp("read", buffer, 4) == 0) {
         get_time(timebuf);
            printf("%s%s System%s: read command requested\n", SYS, timebuf, RES);
            fprintf(log, "%s System: read command requested\n", timebuf);
            fflush(log);
            // appending all postings into a one huge message
            memset(buffer,'\0',BUFSIZE);
            strcpy(buffer, "The following messages have been posted by users:\n");
            for (int i = 0; i != 10; i++)
            {
                strcat(buffer, postings[i]);
                //strcat(msgs, "\n");
            }
            n = write(sock, buffer, BUFSIZE);
             if (n < 0) {
               get_time(timebuf);
               fprintf(log, "%s System: ERROR on writing to socket\n", timebuf);
               fflush(log);
              return 1;
           }
           memset(buffer,'\0',BUFSIZE);
        }
        // status command
        else if ((strncmp("status", buffer, 6) == 0) && (strncmp(user, "admin", 5) == 0)) {
          printf("%s%s System%s: status command requested\n", SYS, timebuf, RES);
          fprintf(log, "%s System: status command requested\n", timebuf);
          fflush(log);

          memset(buffer,'\0',BUFSIZE);
          strcpy(buffer, "********************************\n"
                         "Users online:\n");
          for (int i = 0; i <= cur_user; i++) {
            if (users[i].name[0] != '\0') {
              strcat(buffer, users[i].name);
              strcat(buffer, " ");
              strcat(buffer, users[i].ip);
              strcat(buffer, " ");
              strcat(buffer, users[i].timebuf);
              strcat(buffer, "\n");
            }
          }
          strcat(buffer, "********************************\n");
          n = write(sock, buffer, BUFSIZE);
        }
       else {
        memset(buffer,'\0',BUFSIZE);
        strcpy(buffer, "The command is not recognized. Please, try again");
        n = write(sock, buffer, BUFSIZE);
        if (n < 0) {
          get_time(timebuf);
          fprintf(log, "%s System: ERROR on writing to socket\n", timebuf);
          fflush(log);
         return 1;
           }
           memset(buffer,'\0',BUFSIZE);
       }

    }
    /* disconnect */
    // find and remove the user
    memset(users[usr_id].name, '\0', 20);
    get_time(timebuf);
    printf("%s%s System%s: %s disconnected\n", SYS, timebuf, RES, user);
    fprintf(log, "%s System: %s disconnected\n", timebuf, user);
    fflush(log);
}

int main(int argc, char *argv[])
{
    /* network stuff */
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n, pid;

     // time related stuff
     time_t timer;
     char timebuf[9];
     struct tm* tm_info;

     // log
     FILE *log;
     log = fopen("log.txt", "a+");
     if (log != NULL) {
       get_time(timebuf);
       printf("%s%s System%s: log is started\n", SYS, timebuf, RES);
       fprintf(log, "%s System: log is started\n", timebuf);
       fflush(log);
     }

     if (argc < 2) {
       get_time(timebuf);
       fprintf(log,"%s System: ERROR, no port provided\n", timebuf);
       fflush(log);
       exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
       get_time(timebuf);
       fprintf(log, "%s System: ERROR opening socket\n", timebuf);
       fflush(log);
     }

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
                get_time(timebuf);
                fprintf(log, "%s System: ERROR on binding\n", timebuf);
                fflush(log);
              }


     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) {
        get_time(timebuf);
        fprintf(log, "%s System: ERROR on accept\n", timebuf);
        fflush(log);
      }
      // ip address of a client_processing
      char ipAddress[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(cli_addr.sin_addr), ipAddress, INET_ADDRSTRLEN);

      /* Create child thread */
      pthread_t thr;
      // pack the arguments
      struct args_st arg;
      arg.newsockfd = newsockfd;
      strcpy(arg.ipAddress, ipAddress);
      int t = pthread_create(&thr, NULL, client_processing, (void*) &arg);
      if (t < 0) {
        get_time(timebuf);
        fprintf(log, "%s System: ERROR on thread\n", timebuf);
        fflush(log);
        exit(1);
      }
      else {
        get_time(timebuf);
        printf("%s%s System%s: A thread is assigned to: %s\n", SYS, timebuf, RES, ipAddress);
        fprintf(log, "%s System: A thread is assigned to: %s\n", timebuf, ipAddress);
        fflush(log);
      }
      pthread_detach(thr);


   } /* end of while */

     return 0;
}
