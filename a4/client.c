#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_HOST "iot.dslab.pub.ds.open-cloud.xyz"
#define DEFAULT_PORT "18080"
#define BUFFER_SIZE 1024

void printDebug(const char* message) {
    printf("[DEBUG] %s\n", message);
}

void printHelp() {
    printf("Available commands:\n");
    printf("get            : Retrieve server data\n");
    printf("N name surname reason : Quarantine exit permit\n");
    printf("help           : Prints a help message\n");
    printf("exit           : Exit\n");
}

const char* getEventTypeString(int eventType) {
    switch (eventType) {
        case 0:
            return "boot";
        case 1:
            return "setup";
        case 2:
            return "interval";
        case 3:
            return "button";
        case 4:
            return "motion";
        default:
            return "unknown";
    }
}

void processEventData(const char* buffer) {
    int eventType, lightLevel;
    double temperature;
    time_t timestamp;

    sscanf(buffer, "%d %d %lf %ld", &eventType, &lightLevel, &temperature, &timestamp);

    // Get the current local time
    struct tm* localTime = localtime(&timestamp);
    char timeBuffer[80];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

    printf("---------------------------\n");
    printf("Latest event:\n");
    printf("%s (%d)\n", getEventTypeString(eventType), eventType);
    printf("Temperature is: %.2f\n", temperature / 100.0);
    printf("Light level is: %d\n", lightLevel);
    printf("Timestamp is: %s\n", timeBuffer);
    printf("---------------------------\n");
}

int main(int argc, char* argv[]) {
    const char* host = DEFAULT_HOST;
    const char* port = DEFAULT_PORT;
    int debugMode = 0;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            host = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--debug") == 0) {
            debugMode = 1;
        }
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket (optional)
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    char hostname[] = DEFAULT_HOST; /* Το όνοµα του συστήµατος */
    struct hostent *hostp; /* Η διεύθυνση του συστήµατος */ 
    struct sockaddr_in sa; /* Η διεύθυνση του socket */ 
    hostp = gethostbyname(hostname);
/* Ποιά είναι η διεύθυνση του συστήµατος host */
/* Στη συνέχεια ορίζεται η διεύθυνση του socket σε 3 βήµατα */
    sa.sin_family = AF_INET; /* internet */ 
    sa.sin_port = htons((u_short) 18080); //port number
/* Η κλήση htons() και η συγγενής της htonl() µετατρέπει το short και long
αντίστοιχα όρισµα σύµφωνα µε το “byte ordering” που χρησιµοποιείται στο δίκτυο ώστε
να αντιµετωπίζονται τυχόν προβλήµατα που προξενούνται από το “endianess” των
συστηµάτων. Οι αντίστροφες κλήσεις είναι oι ntohs() και η ntohl() */
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length);
/* Internet address του συστήµατος. Είναι επίσης δυνατό, αν ενδιαφέρει το παρόν
σύστηµα να αποφευχθούν οι κλήσεις gethostname() και gethostbyname() και να
χρησιµοποιηθεί η µορφή
sa.sin_addr.s_addr = htonl(INADDR_ANY);
που σηµαίνει “ΙΡ διεύθυνση του socket είναι οποιαδήποτε διεύθυνση αντιστοιχεί στο παρόν
σύστηµα” */

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server: %s:%s\n", host, port);

    // Send and receive data
    char buffer[BUFFER_SIZE];
    //memset(buffer, 0, sizeof(buffer)); ???

    while (1) {
        // Get user input
        printf("> ");
        //fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin); //pairnei to stdin kai to bazei sto buffer

        // Process user commands
        if (strcmp(buffer, "help\n") == 0) {
            printHelp();
        } else if (strcmp(buffer, "exit\n") == 0) {
            break;
        } else if (strcmp(buffer, "get\n") == 0) {
            // Send 'get' command to server
            if (write(sockfd, "get", strlen("get")) == -1) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }

            if (debugMode) {
                printDebug("Sent 'get' command");
            }

            // Receive server response
            int bytesRead = read(sockfd, buffer, sizeof(buffer) - 1);
            if (bytesRead == -1) {
                perror("Receive failed");
                exit(EXIT_FAILURE);
            }

            buffer[bytesRead] = '\0';

            if (debugMode) {
                printf("[DEBUG] Received data: %s\n", buffer);
            }

            // Process and print the received data
            processEventData(buffer);
        } else {
            // Send quarantine exit permit request
            if (write(sockfd, buffer, strlen(buffer)) == -1) {
                perror("Send failed");
                exit(EXIT_FAILURE);
            }

            if (debugMode) {
                printf("[DEBUG] Sent '%s'\n", buffer);
            }

            // Receive server response
            int bytesRead = read(sockfd, buffer, sizeof(buffer));
            if (bytesRead == -1) {
                perror("Receive failed");
                exit(EXIT_FAILURE);
            }

            if (debugMode) {
                printf("[DEBUG] Received data: %s\n", buffer);
            }

            // Print the server response
            printf("Server response: %s\n", buffer);
        }
    }

    // Close the socket
    close(sockfd);

    return 0;
}
