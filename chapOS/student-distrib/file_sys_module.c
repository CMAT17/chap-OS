#include "file_sys_module.h"
#include "multiboot.h"
#include "lib.h"

static uint32_t file_sys_start;
static boot_block_t* boot_block_ptr;
static uint32_t inode_start;
static uint32_t data_block_start;
static uint32_t directory_index = 0;


void file_sys_init(module_t* file_sys_module)
{
    file_sys_start = file_sys_module->mod_start;
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

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    int file_not_found = 1;
    int i = 0;

    while(file_not_found == 1 && i < MAX_DENTRY_NUM)
    {
        if(strncmp((int8_t*)fname,(int8_t*) boot_block_ptr->dir_entries[i].file_name, FILE_NAME_SIZE) == 0){
            file_not_found = 0;
            //copy file name
            strncpy((int8_t*)dentry->file_name, (int8_t*)fname, FILE_NAME_SIZE);

            //copy file type
            dentry->file_type = boot_block_ptr->dir_entries[i].file_type;

            //copy inode number
            dentry->inode_num = boot_block_ptr->dir_entries[i].inode_num;
        }
        i++;
    }
    return (file_not_found == 1 ? -1 : 0); 
}

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
    if(index>=MAX_DENTRY_NUM)
    {
        return -1;
    }

    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block_ptr->dir_entries[index].file_name, FILE_NAME_SIZE);
    dentry->file_type = boot_block_ptr->dir_entries[index].file_type;
    dentry->inode_num = boot_block_ptr->dir_entries[index].inode_num;

    return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    int i;
    uint32_t cur_block_index;
    uint32_t loc_cur_block_index;
    uint32_t cur_data_block;
    inode_t* inode_data_ptr = (inode_t *) (inode_start + (inode)*DATA_BLOCK_SIZE);
    
    if(inode >= boot_block_ptr->num_inodes)
    {
        return -1;
    }

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

int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
    return 0;
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

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
    dentry_t dir_entry;
    int32_t i;

    //clear buffer
    for(i = 0; i<=FILE_NAME_SIZE+1; i++){
        ((int8_t*)(buf))[i] = '\0';
    }
    if(read_dentry_by_index( directory_index,&dir_entry) == 0)
    {
        //dentry can be read by index
        uint32_t length = strlen((int8_t *) dir_entry.file_name);
        strncpy((int8_t *)buf, (int8_t *) dir_entry.file_name, length);
        directory_index++;
        return length;
    }
    else
    {
        directory_index = 0;
        return 0;
    }
}
