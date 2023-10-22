#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 3001

pthread_mutex_t myMutex;
pthread_t thread[100];

int client_sockets[100];
int total_clients=0;

char usernames[3][20];
char passwords[3][20];
char commands[5][50]={"direct\0","group\0","history\0","creategroup\0","joingroup\0"};

int active_clients_sockets[100];
char active_clients_names[10][100];
int total_active_clients=0;

int active_indexes[10];

struct group
{
    char name[100];
    char members[10][100];
    char history[10][100];
    int history_count;
    int number_of_members;
};

struct group groups[10];

int group_count=0;


void *client_handler(void *arg) 
{
    int client_socket = *(int *)arg;
    char buffer[1024];
    int bytes_received;

    char username_msg[1024] = "Please enter username";
    char password_msg[1024] = "Please enter password";

    bool flag=0;
    int pos=0;
    while (1) 
    {
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, username_msg);
        send(client_socket, buffer, strlen(buffer), 0);
        int bytes_received=recv(client_socket, buffer, sizeof(buffer), 0);
        buffer[bytes_received-1] = '\0'; 

        if (strcmp(buffer,"exit\0") == 0) 
        {
            printf("Client %d disconnected!\n",client_socket);
            close(client_socket);
            return 0;
        }


        strcpy(username_msg, "Username incorrect, please try again!");
        for(int i=0;i<3;i++)
        {
            if (strcmp(buffer,usernames[i]) == 0) 
            {
                flag=1;
                pos=i;

                printf("Username present!\n");
                for(int j=0;j<10;j++)
                {
                    if (strcmp(buffer,active_clients_names[i]) == 0) 
                    {
                        strcpy(username_msg, "Username already active, try again with another username!\0");
                        flag=0;
                    }
                }

            } 
        }
        if(flag==1)
            break;
    }

    while (1) 
    {
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, password_msg);
        send(client_socket, buffer, strlen(buffer), 0);
        int bytes_received=recv(client_socket, buffer, sizeof(buffer), 0);
        buffer[bytes_received-1] = '\0'; 
        // printf("%s",buffer);

        if (strcmp(buffer,"exit\0") == 0) 
        {
            printf("Client %d disconnected!\n",client_socket);
            close(client_socket);
            return 0;
        }

        if (strcmp(buffer,passwords[pos]) == 0) 
        {
            printf("Password present!\n");
            memset(buffer,0,sizeof(buffer));
            strcpy(buffer,"Valid username and password!\nPlease message anyone you like!");
            send(client_socket, buffer, strlen(buffer), 0);
            break;
        } 
        else
        {
            strcpy(password_msg, "Password incorrect, please try again!");
        }
    }

    int active_index=-1;
    pthread_mutex_lock(&myMutex);
    for(int i=0;i<10;i++)
    {
        if(active_indexes[i]==0)
        {
            active_index=i;
            active_clients_sockets[active_index]=client_socket;
            strcpy(active_clients_names[active_index],usernames[pos]);
            active_indexes[active_index]=1;
            break;

        }
    }
    pthread_mutex_unlock(&myMutex);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) 
        {
            break;
        }

        buffer[bytes_received-1] = '\0'; 
        if (strcmp(buffer,"exit\0") == 0) 
        {
            printf("Client %d disconnected!\n",client_socket);
            pthread_mutex_lock(&myMutex);
            active_indexes[active_index]=0;
            strcpy(active_clients_names[active_index],"");
            pthread_mutex_unlock(&myMutex);
            break;
        }

        int option=0;
        bool userpresent=0;
        bool useractive=0;
        int client_socket_receiver=-1;
        int group_number=0;
        bool group_present=0;
        int user_present=0;

        char original_buffer[100];
        strcpy(original_buffer, buffer);

        char *token = strtok(buffer, " ");

        if(token==NULL)
        {
            strcpy(buffer,"Invalid format\0");
            send(client_socket,buffer, sizeof(buffer), 0);
            continue;
        }

        if(strcmp(token,commands[0])==0)
        {
            option=1;
            token = strtok(NULL, " ");
            if(token==NULL)
            {
                strcpy(buffer,"Invalid format\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            for(int i=0;i<3;i++)
            {
                if(strcmp(token,usernames[i])==0)
                {
                    userpresent=1;

                }
            }
            for(int j=0;j<10;j++)
            {
                if(strcmp(token,active_clients_names[j])==0)
                {
                    useractive=1;
                    client_socket_receiver=active_clients_sockets[j];
                }
            }
        }
        else if(strcmp(token,commands[1])==0)
        {
                option=2;
                token = strtok(NULL, " ");
                for(int i=0;i<10;i++)
                {
                    if(strcmp(token,groups[i].name)==0)
                    {
                       group_number=i;
                       group_present=1;
                       for(int j=0;j<groups[i].number_of_members;j++)
                       {
                            if(strcmp(usernames[pos],groups[i].members[j])==0)
                            {
                                user_present=1;
                            }
                       }
                    }
                }
        }
        else if(strcmp(token,commands[2])==0)
        {
                option=3;
                // printf("%s\n",token);
                token = strtok(NULL, " ");
                // printf("%s\n",token);
                for(int i=0;i<10;i++)
                {
                    if(strcmp(token,groups[i].name)==0)
                    {
                       group_number=i;
                       group_present=1;
                    }
                }
        }
        else if(strcmp(token,commands[3])==0)
        {
            // CREATING A NEW CHAT GROUP
            option=4;
            token = strtok(NULL, " ");
            // printf("%s \n",token);
            if(token==NULL)
            {
                strcpy(buffer,"Invalid format\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            for(int i=0;i<group_count;i++)
            {
                if(strcmp(token,groups[i].name)==0)
                {
                    group_present=1;
                }
            }
            if(group_present==1)
            {
                strcpy(buffer,"A group with that name is already present!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            else
            {
                pthread_mutex_lock(&myMutex);
                strcpy(groups[group_count].name,token);
                groups[group_count].history_count=0;
                groups[group_count].number_of_members=0;
                group_count+=1;
                pthread_mutex_unlock(&myMutex);

                char msg[100];
                strcpy(msg,"Group created with the name : ");
                strcat(msg,token);
                send(client_socket,msg, sizeof(msg), 0);
                continue;

            }
        }
        else if(strcmp(token,commands[4])==0)
        {
            int group_no=-1;

            // JOINING A CHAT GROUP
            token = strtok(NULL, " ");
            // printf("%s \n",token);
            if(token==NULL)
            {
                strcpy(buffer,"Invalid format\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            for(int i=0;i<group_count;i++)
            {
                if(strcmp(token,groups[i].name)==0)
                {
                    group_present=1;
                    group_no=i;
                    printf("%s\n",groups[i].name);
                }
            }
            if(group_present==1)
            {
                int client_present=0;
                for(int i=0;i<groups[group_no].number_of_members;i++)
                {
                    if(strcmp(usernames[pos],groups[group_no].members[i])==0)
                    {
                        client_present=1;
                    }
                }

                if(client_present==1)
                {
                    strcpy(buffer,"You have already joined this group!\0");
                    send(client_socket,buffer, sizeof(buffer), 0);
                    continue;
                }

                pthread_mutex_lock(&myMutex);
                strcpy(groups[group_no].members[groups[group_no].number_of_members],usernames[pos]);
                groups[group_no].number_of_members+=1;
                pthread_mutex_unlock(&myMutex);

                char msg[100];
                strcpy(msg,"You joined the group : ");
                strcat(msg,token);
                send(client_socket,msg, sizeof(msg), 0);
                continue;

            }
            else
            {
                strcpy(buffer,"No group with this name is present!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }



        }
        else
            option=-1;


        if(option==1)
        {
            //direct msg
            if(userpresent==1)
            {
                if(useractive==1)
                {
                    char message[100];
                    for(int i=0;i<100;i++)
                        message[i]='\0';

                    char inital_stmt[50]="Message from \0";

                    strcpy(message,inital_stmt);
                    strcat(message,active_clients_names[pos]);
                    strcat(message," : \0");   
                    
                    while(1)
                    {
                        token = strtok(NULL, " ");
                        if(token==NULL)
                        {
                            strcat(message,"\0");   
                            break;
                        }
                        else
                        {
                            strcat(message," "); 
                            strcat(message,token); 
                        }
                    }
                    send(client_socket_receiver,message, sizeof(message), 0);
                }
                else
                {
                    strcpy(buffer,"user not active!\0");
                    send(client_socket,buffer, sizeof(buffer), 0);
                }
            }
            else
            {
                strcpy(buffer,"no user by that name!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
            }
        }
        else if(option==2)
        {
            //gp msg
            if(group_present==0)
            {
                strcpy(buffer,"no group by that name!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }

            if(user_present==0)
            {
                strcpy(buffer,"You are not part of this group-chat!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            
            char message[100];
            for(int i=0;i<100;i++)
                message[i]='\0';

            char inital_stmt[50]="Message from \0";

            strcpy(message,inital_stmt);
            strcat(message,active_clients_names[pos]);
            strcat(message," in group \0");
            strcat(message,groups[group_number].name);
            strcat(message," : \0");   
            
            while(1)
            {
                token = strtok(NULL, " ");
                if(token==NULL)
                {
                    strcat(message,"\0");   
                    break;
                }
                else
                {
                    strcat(message," "); 
                    strcat(message,token); 
                }
            }

            pthread_mutex_lock(&myMutex);
            strcpy(groups[group_number].history[groups[group_number].history_count],message);
            groups[group_number].history_count+=1;
            pthread_mutex_unlock(&myMutex);

            for(int i=0;i<groups[group_number].number_of_members;i++)
            {
                for(int j=0;j<3;j++)
                {
                    if(strcmp(groups[group_number].members[i],active_clients_names[j])==0)
                    {
                        send(active_clients_sockets[j],message, sizeof(message), 0);
                    }
                }
            }
        }
        else if(option==3)
        {
            //gp message history
            if(group_present==0)
            {
                strcpy(buffer,"no group by that name!\0");
                send(client_socket,buffer, sizeof(buffer), 0);
                continue;
            }
            else
            {
                for(int i=0;i<groups[group_number].history_count;i++)
                {
                    printf("\n");
                    send(client_socket,groups[group_number].history[i],sizeof(groups[group_number].history[i]),0);
                }
            }
        }
        else if(option==4)
        {
            
        }
        else
        {
            strcpy(buffer,"invalid command!\n");
            send(client_socket,buffer, sizeof(buffer), 0);
            continue;
        }

        // Send data to the other client (assuming the buffer contains the client socket number)
    }
    // Close the client socket when communication is done
    close(client_socket);

}

int main()
{
    strcpy(usernames[0], "hamza\0");
    strcpy(usernames[1], "ahmed\0");
    strcpy(usernames[2], "ali\0");

    strcpy(passwords[0], "hamza\0");
    strcpy(passwords[1], "ahmed\0");
    strcpy(passwords[2], "ali\0");

    strcpy(groups[0].name, "funchat\0");
    strcpy(groups[0].members[0], "hamza\0");
    strcpy(groups[0].members[1], "ahmed\0");
    strcpy(groups[0].members[2], "ali\0");

    groups[0].number_of_members=3;

    groups[0].history_count=0;
    group_count+=1;

    for(int i=0;i<10;i++)
    {
        active_indexes[i]=0;
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];
    
    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // Initialize the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) 
    {

        perror("Error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening");
        exit(1);
    }
    
    while (1) 
    {
        // client_socket=accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        client_sockets[total_clients] = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        client_socket = client_sockets[total_clients];

        printf("Client %d connected\n", client_socket);

        pthread_mutex_init(&myMutex, NULL);

        pthread_create(&thread[total_clients], NULL, client_handler, &client_sockets[total_clients]);
        pthread_detach(&thread[total_clients]);

        total_clients+=1;

        pthread_mutex_destroy(&myMutex);

    }

    // Close the sockets
    close(server_socket);
    return 0;
}