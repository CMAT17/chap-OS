#include "file_sys_module.h"
#include "multiboot.h"
#include "lib.h"
#include "system_call.h"


static uint32_t file_sys_start;
static boot_block_t* boot_block_ptr;
static uint32_t inode_start;
static uint32_t data_block_start;
static uint32_t directory_index = 0;


void file_sys_init(module_t* file_sys_module)
{
    file_sys_start = file_sys_module->mod_start;
    printf("%d\n",file_sys_start);
    inode_start = file_sys_start + DATA_BLOCK_SIZE;
    boot_block_ptr = (boot_block_t *)file_sys_start;
    data_block_start = file_sys_start + (1+boot_block_ptr->num_inodes)*DATA_BLOCK_SIZE;
}

int32_t file_sys_open(const uint8_t * filename)
{
    return 0;
}

int32_t file_sys_close(int32_t fd)
{
    return 0;
}

int32_t file_sys_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * read_dentry_by_name
 *         Searches filesystem for given filename and if found copies
 *         file information into dentry
 * INPUT: fname - filename to be searched for in filesystem
 *        dentry - filled in with file name, file type, and inode number
 * OUTPUT: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    int file_not_found = FILE_NOT_FOUND;
    int i = 0;

    //iterate through directory entries until fname matches the dentry filename
    while(file_not_found == 1 && i < MAX_DENTRY_NUM)
    {
        if(strncmp((int8_t*)fname,(int8_t*) boot_block_ptr->dir_entries[i].file_name, FILE_NAME_SIZE) == 0){
            file_not_found = FILE_FOUND;
            //copy file name
            strncpy((int8_t*)dentry->file_name, (int8_t*)fname, FILE_NAME_SIZE);

            //copy file type
            dentry->file_type = boot_block_ptr->dir_entries[i].file_type;

            //copy inode number
            dentry->inode_num = boot_block_ptr->dir_entries[i].inode_num;
        }
        i++;
    }

    //return -1 if file is nonexistent 
    return (file_not_found == FILE_NOT_FOUND ? -1 : 0); 
}

/*
 * read_dentry_by_index
 * INPUT: index - index of directory entry
 *        dentry - filled in with file name, file type, and inode number
 * OUTPUT: none
 * RETURN VALUE: 0 on success, -1 on failure
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    //check if invalid index
    if(index>=MAX_DENTRY_NUM)
    {
        return -1;
    }

    //check if filename is actually valid and not empt
    if((int8_t)boot_block_ptr->dir_entries[index].file_name[0]=='\0')
    {
        return -1;
    }
    ///fills in dentry with file name, type, and inode number
    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block_ptr->dir_entries[index].file_name, FILE_NAME_SIZE);

    dentry->file_type = boot_block_ptr->dir_entries[index].file_type;
    dentry->inode_num = boot_block_ptr->dir_entries[index].inode_num;

    return 0;
}

/*
 * read_data
 * INPUT: inode - index node
 *        offset - starting position in file
 *        buf - data stored in this buffer
 *        length - read up to this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read, 0 indicatees end of file, -1 on failure
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    int i;
    uint32_t cur_block_index;
    uint32_t loc_cur_block_index;
    uint32_t cur_data_block;
    inode_t* inode_data_ptr = (inode_t *) (inode_start + (inode)*DATA_BLOCK_SIZE);
    
    //check if given inode number is in range
    if(inode >= boot_block_ptr->num_inodes)
    {
        return -1;
    }
    //no bytes will be read because offset is greater than the length
    if(offset >= inode_data_ptr->length)
    {
        return 0;
    }
    

    cur_block_index = offset/DATA_BLOCK_SIZE;
    loc_cur_block_index = offset % DATA_BLOCK_SIZE;
    cur_data_block = data_block_start+(inode_data_ptr->data_block[cur_block_index]) * DATA_BLOCK_SIZE;

    i = 0;
    while(i < length && i+offset <= inode_data_ptr->length)
    {
        if(loc_cur_block_index == DATA_BLOCK_SIZE)
        {
            cur_block_index++;
            loc_cur_block_index = 0;
            cur_data_block = data_block_start+(inode_data_ptr->data_block[cur_block_index]) * DATA_BLOCK_SIZE;
        }

        //checks if bad data block number is found within the file bounds of inode
        if(inode_data_ptr->data_block[cur_block_index] >= boot_block_ptr->num_data_blocks)
        {
            return -1;
        }

        buf[i] = (uint8_t) ((uint8_t*)cur_data_block)[loc_cur_block_index];
        i++;
        loc_cur_block_index++;
    }
    return i;
}

int32_t file_open(const uint8_t * filename)
{
    return 0;
}

int32_t file_close(int32_t fd)
{
    return 0;
}

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}

/*
 * file_read
 * INPUT: fd - file descriptor in array of pcb
 *        buf - data stored in this buffer
 *        nbytes - read this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
    uint32_t offset, inode_num;
    int32_t nbytes_read;
    pcb_t* cur_pcb;
    cur_pcb = get_pcb_ptr();
    
    inode_num = cur_pcb->f_descs[fd].inode;
    offset = cur_pcb->f_descs[fd].file_pos;
    nbytes_read = read_data(inode_num, offset, (uint8_t *) buf, nbytes);

    cur_pcb->f_descs[fd].file_pos += nbytes_read;
    return nbytes_read;
}

int32_t dir_open(const uint8_t * filename)
{
    return 0;
}

int32_t dir_close(int32_t fd)
{
    return 0;
}

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return -1;
}


/*
 * dir_read
 * INPUT: fd - file descriptor in array of pcb
 *        buf - data stored in this buffer
 *        nbytes - read this number of bytes
 * OUTPUT: none
 * RETURN VALUE: number of bytes read, 0 indicatees end of file
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
    dentry_t dir_entry;
    //int32_t i;
    
    //clear buffer
    /*for(i = 0; i<FILE_NAME_SIZE+1; i++){
        ((int8_t*)(buf))[i] = '\0';
    }*/
    int32_t test = read_dentry_by_index(directory_index, &dir_entry);
    if(test == 0)
    {
        //dentry can be read by index
        //uint32_t length = strlen((int8_t *) dir_entry.file_name);
        //strncpy((int8_t *)buf, (int8_t *) dir_entry.file_name, length);
        memcpy(buf, (void*)dir_entry.file_name, FILE_NAME_SIZE);
        directory_index++;
        return FILE_NAME_SIZE;
    }
    else
    {
        directory_index = 0;
        return 0;
    }
}

/*
 * get_file_size
 * INPUT: inode_num - number of inode
 * OUTPUT: none
 * RETURN VALUE: length in bytes of file
 */
uint32_t get_file_size(uint32_t inode_num)
{
    inode_t * f_inode;
    f_inode = (inode_t*)(inode_start + inode_num*DATA_BLOCK_SIZE);
    return f_inode->length;
}





