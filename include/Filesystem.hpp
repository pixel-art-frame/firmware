#pragma once

typedef enum {

    UNMOUNTED = 0,
    MOUNTED,
    MOUNT_FAILED

} sd_state_t;


bool handle_sd_error();
bool mount_fs();