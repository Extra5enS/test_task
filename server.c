
        /* Add logic with socket
         * 
         * For a seekable file (i.e., one to which lseek(2) may be applied, for example, a regular file) writing takes place at the file offset, \
         * and the file offset is incremented by the number  of bytes  actually  written.   
         * If  the file was open(2)ed with O_APPEND, the file offset is first set to the end of the file before writing.  
         * The adjustment of the file offset and the write operation are performed as an atomic step. */
