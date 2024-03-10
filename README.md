# SC3020

#### Installation Guide
Download ZIP folder from the GitHub link provided in Section 4 (Source Code)  
For MacOS, Build and compile the cpp files in the terminal using the command below.  
g++ -std=c++11 main.cpp storage.cpp bplustree.cpp -o main.exe  
Run the main Unix executable file using ./main  

For Windows, Build and compile the cpp files in the terminal using the command below.  
g++ -g -fdiagnostics-color=always ‘Filepath to main.cpp’ ‘Filepath to BPlusTree.cpp’
‘Filepath to storage.cpp’ - o ‘Filepath to output main.exe’  
Example:  
g++ -g -fdiagnostics-color=always C:\Users\lohsh\OneDrive\Documents\GitHub\SC3020\main.cpp C:\Users\lohsh\OneDrive\Documents\GitHub\SC3020\BPlusTree.cpp C:\Users\lohsh\OneDrive\Documents\GitHub\SC3020\storage.cpp
 -o C:\Users\lohsh\OneDrive\Documents\GitHub\SC3020\main.exe  
Run the main executable file
