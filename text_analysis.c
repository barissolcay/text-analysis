#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#define INPUT_CHARACTER_LIMIT 100 // Define the maximum length for input strings
#define OUTPUT_CHARACTER_LIMIT 10000 // Define the maximum length for output responses
#define PORT_NUMBER 60000 // Define the port number for the server to listen on
#define LEVENSHTEIN_LIST_LIMIT 5 // Define the maximum number of closest words to return

// Function to find the minimum of three integers
int min(int a, int b, int c) {
    if (a <= b && a <= c) {
        return a; // Return a if it is the smallest
    } else if (b <= c) {
        return b; // Return b if it is the smallest
    } else {
        return c; // Otherwise, return c
    }
}

// Function to calculate the Levenshtein distance between two strings
int levenshtein(const char *s1, const char *s2) {
    unsigned int x, y, s1len, s2len;
    s1len = strlen(s1); // Get the length of the first string
    s2len = strlen(s2); // Get the length of the second string

    // Create a matrix to store distances
    unsigned int matrix[s2len + 1][s1len + 1];

    // Initialize the first column of the matrix
    for (x = 0; x <= s2len; x++) {
        matrix[x][0] = x; // Distance from s2 to an empty s1
    }

    // Initialize the first row of the matrix
    for (y = 0; y <= s1len; y++) {
        matrix[0][y] = y; // Distance from s1 to an empty s2
    }

    // Fill the matrix with distances calculated by the Levenshtein algorithm
    for (x = 1; x <= s2len; x++) {
        for (y = 1; y <= s1len; y++) {
            int cost = (s1[y - 1] == s2[x - 1]) ? 0 : 1; // Cost of substitution
            matrix[x][y] = min(
                matrix[x - 1][y] + 1, // Cost of deletion
                matrix[x][y - 1] + 1, // Cost of insertion
                matrix[x - 1][y - 1] + cost // Cost of substitution
            );
        }
    }

    return matrix[s2len][s1len]; // Return the computed Levenshtein distance
}

// Function to find the closest words in the dictionary based on distances
void find_closest_words(const char *word, char dictionary[2000][50], int dict_count, char closest_words[5][50], int distances[5]) {
    // Initialize distances array with a maximum value
    for (int i = 0; i < LEVENSHTEIN_LIST_LIMIT; i++) {
        distances[i] = INT_MAX; // Set all distances to a high value
    }

    // Compare the input word with each word in the dictionary
    for (int i = 0; i < dict_count; i++) {
        int dist = levenshtein(word, dictionary[i]); // Calculate Levenshtein distance

        // Update closest words and distances if a closer match is found
        for (int j = 0; j < LEVENSHTEIN_LIST_LIMIT; j++) {
            // Check if the current distance is smaller than the known distance
            if (dist < distances[j] || (dist == distances[j] && strcmp(dictionary[i], closest_words[j]) < 0)) {
                // Shift existing closest words and distances to make space for the new closest word
                for (int k = LEVENSHTEIN_LIST_LIMIT - 1; k > j; k--) {
                    distances[k] = distances[k - 1]; // Shift distance
                    strcpy(closest_words[k], closest_words[k - 1]); // Shift word
                }
                distances[j] = dist; // Update distance
                strcpy(closest_words[j], dictionary[i]); // Update closest word
                break; // Exit the loop as we have inserted the new closest word
            }
        }
    }
}

// Function to process the input string from the client
void process_input(char *input, char words[INPUT_CHARACTER_LIMIT][50], int *word_count) {
    *word_count = 0; // Initialize word count to zero
    char *token; // Pointer to store each word

    // Convert input string to lowercase for uniformity
    for (int i = 0; input[i]; i++) {
        input[i] = tolower(input[i]); // Convert each character to lowercase
    }

    // Split the input string into words based on spaces
    token = strtok(input, " "); // Get the first token
    while (token != NULL && *word_count < INPUT_CHARACTER_LIMIT) {
        strcpy(words[*word_count], token); // Store the token in the words array
        (*word_count)++; // Increment the word count
        token = strtok(NULL, " "); // Get the next token
    }
}

// Function to ensure the dictionary file ends with a newline character
void ensure_file_format(const char *filename) {
    FILE *file = fopen(filename, "a+"); // Open file in append mode
    if (file == NULL) {
        return; // Return if file cannot be opened
    }

    // Check if the last character in the file is a newline character
    fseek(file, -1, SEEK_END); // Move the file pointer to the end
    char last_char = fgetc(file); // Read the last character
    if (last_char != '\n') {
        fprintf(file, "\n"); // If not a newline, write one
    }

    fclose(file); // Close the file
}

// Function to add a new word to the dictionary
void add_word_to_dictionary(const char *word, char dictionary[2000][50], int *dict_count, const char *filename) {
    strcpy(dictionary[*dict_count], word); // Add the new word to the dictionary array
    (*dict_count)++; // Increment the count of dictionary words

    // Append the new word to the dictionary file
    FILE *file = fopen(filename, "a"); // Open the file in append mode
    if (file != NULL) {
        fprintf(file, "%s\n", word); // Write the new word to the file
        fclose(file); // Close the file
    }
}

int main() {
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client; // Define structures for server and client addresses
    char client_message[INPUT_CHARACTER_LIMIT + 1]; // Buffer to receive messages from the client
    char client_message_temp[INPUT_CHARACTER_LIMIT + 1]; // Backup for the original message
    char words[INPUT_CHARACTER_LIMIT][50]; // Array to hold words extracted from the input
    int word_count; // Variable to hold the count of processed words
    char result_string[OUTPUT_CHARACTER_LIMIT] = ""; // String to store final output

    // Open the dictionary file containing words
    FILE *file = fopen("basic_english_2000.txt", "r"); // Try to open the dictionary file
    if (file == NULL) {
        printf("ERROR: Dictionary file “basic_english_2000.txt” not found!\n");
        return 1; // Exit if the file cannot be found
    }

    char dictionary[2000][50]; // Array to store words from the dictionary
    int dict_count = 0; // Count of words read from the dictionary
    while (fgets(dictionary[dict_count], sizeof(dictionary[dict_count]), file)) {
        // Remove any newline characters from each word read
        dictionary[dict_count][strcspn(dictionary[dict_count], "\r\n")] = '\0';
        if (strlen(dictionary[dict_count]) > 0) {
            dict_count++; // Increment the dictionary word count if the word is not empty
        }
    }
    fclose(file); // Close the dictionary file

    // Convert all dictionary words to lowercase for consistency
    for (int i=0; i<dict_count; i++) {
         for (int j=0; dictionary[i][j]; j++) {
            dictionary[i][j] = tolower(dictionary[i][j]); // Convert each word to lowercase
        }
    }
    
    // Create a socket for the server
    socket_desc = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (socket_desc == -1) {
        printf("ERROR: Could not create socket\n");
        return 1; // Exit if socket creation fails
    }
    printf("Socket created successfully\n");

    // Set up the server address structure
    server.sin_family = AF_INET; // Set family to IPv4
    server.sin_addr.s_addr = INADDR_ANY; // Allow connections from any IP address
    server.sin_port = htons(PORT_NUMBER); // Set the port number

    // Bind the socket to the specified address and port
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("ERROR: Binding failed");
        return 1; // Exit if binding fails
    }
    printf("Binding successful\n");

    // Start listening for incoming connections
    listen(socket_desc, 3); // Listen for up to 3 connections in the queue
    printf("Waiting for incoming connections on port %d...\n", PORT_NUMBER);

    c = sizeof(struct sockaddr_in); // Get the size of the client structure
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c); // Accept a client connection

    if (client_sock < 0) {
        perror("ERROR: Accept failed");
        return 1; // Exit if accepting fails
    }
    printf("Connection accepted\n");

    // Send a welcome message to the client
    send(client_sock, "Hello, this is Text Analysis Server!\nPlease enter your input string:\n", 72, 0);

    // Receive input from the client
    if (recv(client_sock, client_message, INPUT_CHARACTER_LIMIT, 0) < 0) {
        perror("ERROR: Failed to receive message");
        return 1; // Exit if receiving fails
    }
    client_message[strcspn(client_message, "\r\n")] = '\0'; // Remove any trailing newline characters
    strcpy(client_message_temp, client_message); // Backup the original input for final response
    printf("Client message received: %s\n", client_message);

    // Check if the input string is empty
    if (strlen(client_message) == 0) {
        send(client_sock, "ERROR: Empty input received!\n", 29, 0); // Error response for empty input
        close(client_sock); // Close the client socket
        close(socket_desc); // Close the server socket
        return 1; // Exit the program
    }

    // Check if the input string exceeds the defined character limit
    if (strlen(client_message) > INPUT_CHARACTER_LIMIT) {
        printf("ERROR: Input string is longer than %d characters!\n", INPUT_CHARACTER_LIMIT);
        close(client_sock); // Close the client socket
        close(socket_desc); // Close the server socket
        return 1; // Exit the program
    }

    // Process the input string and split it into words
    process_input(client_message, words, &word_count); // Call the function to process input

    // For each word received from the client
    for (int i = 0; i < word_count; i++) {
        result_string[0] = '\0'; // Reset the result string for each new word
        char closest_words[5][50]; // Array to store closest words
        int distances[5]; // Array to store the distances of the closest words
        find_closest_words(words[i], dictionary, dict_count, closest_words, distances); // Find closest words

        // Create a result string for the current word
        char result[OUTPUT_CHARACTER_LIMIT] = {0}; // Reset result buffer
        snprintf(result, sizeof(result), "WORD %02d: %s\nMATCHES: ", i + 1, words[i]);
        
        // Add closest matches to the result string
        for (int j = 0; j < LEVENSHTEIN_LIST_LIMIT; j++) {
            char match[100];
            snprintf(match, sizeof(match), "%s (%d), ", closest_words[j], distances[j]); // Format the match
            strncat(result, match, sizeof(result) - strlen(result) - 1); // Append match to result string safely
        }
        strcat(result, "\n"); // Add a newline character after matches
        send(client_sock, result, strlen(result), 0); // Send the result to the client

        // Check if the word exists in the dictionary
        int found = 0; // Flag to check if the word is found in the dictionary
        for (int j = 0; j < dict_count; j++) {
            // Compare the current word with dictionary words
            if (strcmp(words[i], dictionary[j]) == 0) {
                found = 1; // Set found flag if match is found
                break; // Exit loop once found
            }
        }

        // If the word is not found in the dictionary, prompt user to add it
        if (!found) {
            send(client_sock, "Word not in dictionary. Add it? (y/N): ", 38, 0);
            char response[5]; // Buffer to hold user response
            memset(response, 0, sizeof(response)); // Clear the response buffer
            recv(client_sock, response, sizeof(response), 0); // Receive response from the client
            response[strcspn(response, "\r\n")] = '\0'; // Clean up response for newlines
            
            // If user wants to add the word
            if (tolower(response[0]) == 'y') {
                add_word_to_dictionary(words[i], dictionary, &dict_count, "basic_english_2000.txt"); // Add word to dictionary
                send(client_sock, "Word added to dictionary.\n", 26, 0); // Confirm addition to client
            } else {
                send(client_sock, "Word not added to dictionary.\n", 30, 0); // Confirm word not added
                strcpy(words[i], closest_words[0]); // Suggest the closest word to the user
            }
        }
    }

    // Create the final result string with all processed words
    for (int i = 0; i < word_count; i++) {
        strncat(result_string, words[i], sizeof(result_string) - strlen(result_string) - 1); // Append words to result string
        if (i < word_count - 1) {
            strncat(result_string, " ", sizeof(result_string) - strlen(result_string) - 1); // Add space between words if not the last word
        }
    }

    // Prepare the final message to send to the client
    char final_message[OUTPUT_CHARACTER_LIMIT]; // Buffer for final message
    snprintf(final_message, sizeof(final_message),
             "INPUT: %.1000s\nOUTPUT: %.1000s\nThank you for using Text Analysis Server! Good Bye!\n",
             client_message_temp, result_string); // Format the final message

    // Send the final message to the client
    send(client_sock, final_message, strlen(final_message), 0);

    // Close the client and server sockets after communication ends
    close(client_sock); // Close the client socket
    close(socket_desc); // Close the server socket

    return 0; // Exit the program successfully
}