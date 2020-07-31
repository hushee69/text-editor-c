# text-editor-c

Simple text editor in C that reads indefinitely and saves to the specified file at the end.

Options for Text Editor

    -a => display content
  
    -e => edit indefinitely
  
    -m => modify a line
  
    -f => file to modify
    
    -l => line to modify
    
    -c => column to modify
    
    -d => delete line
 
 Options for Vignere Cipher
 
    -c => crypte file
    
    -d => decrypt fie
    
    -k => file with the key to use
    
    -f => file to encrypt / decrypt

### Compilation

    gcc -o editor_text.c
    
    gcc -o vignere vignere.c
 
### Remarks:
  
  The text editor can be piped with Vignere and other terminal programs
