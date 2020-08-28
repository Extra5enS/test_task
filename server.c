
/* Add logic with socket
 * 
 * For a seekable file (i.e., one to which lseek(2) may be applied, for example, a regular file) writing takes place at the file offset, \
 * and the file offset is incremented by the number  of bytes  actually  written.   
 * If  the file was open(2)ed with O_APPEND, the file offset is first set to the end of the file before writing.  
 * The adjustment of the file offset and the write operation are performed as an atomic step.
 *
 * A successful return from write() does not make any guarantee that data has been committed to disk.  
 * On some filesystems, including NFS, it does not even guarantee that space has success‐fully  been reserved for the data.  
 * In this case, some errors might be delayed until a future write(2), fsync(2), or even close(2).  
 * The only way to be sure is to call fsync(2) after you are done writing all your data.*/

