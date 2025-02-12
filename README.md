# Text Analysis

Text Analysis is a project that provides a server to analyze input text, identify words, and suggest the closest matches from a dictionary. The system uses the Levenshtein distance algorithm to find the closest words and allows adding new words to the dictionary.

## Project Overview

Text Analysis is developed using C and provides functionalities for receiving text input from clients, processing the text to identify words, and suggesting the closest matches from a predefined dictionary.

### Features

The project includes:
1. **Levenshtein Distance Calculation**: Calculates the Levenshtein distance between two strings to find the closest matches.
2. **Dictionary Management**: Allows adding new words to the dictionary.
3. **Socket Communication**: Uses sockets to receive text input from clients and send back results.
4. **Text Processing**: Processes input text to identify and analyze words.

## Technologies Used

- **C**: The primary programming language used to develop the text analysis system.
- **Sockets**: Used for communication between the server and clients.

## Project Structure

The project structure includes several files organized as follows:

- **`text_analysis.c`**: The main file that implements the text analysis server.
- **`basic_english_2000.txt`**: The dictionary file containing a list of basic English words.

## Usage

### Prerequisites

To run the project, you need to have the following installed:
- **C Compiler**: Ensure you have a C compiler installed on your system.

### Running the Project

1. Clone the repository:

    ```bash
    git clone https://github.com/barissolcay/text-analysis.git
    cd text-analysis
    ```

2. Compile the C file:

    ```bash
    gcc text_analysis.c -o text_analysis
    ```

3. Run the Text Analysis server:

    ```bash
    ./text_analysis
    ```

    The server will start and listen for incoming connections on port 60000.

### Connecting to the Server

You can use a client such as `telnet` to connect to the server:

```bash
telnet localhost 60000
