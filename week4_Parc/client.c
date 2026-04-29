#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
    const char *hostname = "www.naver.com";
    struct hostent *host_info;

    host_info = gethostbyname(hostname);

    if(host_info == NULL){
        fprintf(stderr, "Error: Unable to resolve hostname %s\n", hostname);
        return 1;
    }

    printf("Hostname : %s\n", host_info->h_name);
    printf("IP Address : %s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr_list[0])); //network to ascii

    if(host_info->h_aliases[0] != NULL) {
        printf("Aliases : \n");
        for(int i=0;host_info->h_aliases[i];i++){
            printf(" %s\n", host_info->h_aliases[i]);
        }
    }
    else {
        printf("No aliases found.\n");
    }

    return 0;

}