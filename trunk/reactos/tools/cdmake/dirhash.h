#ifndef _REACTOS_TOOLS_CDMAKE_DIRHASH_H_
#define _REACTOS_TOOLS_CDMAKE_DIRHASH_H_

#define NUM_DIR_HASH_BUCKETS 1024

struct target_file {
    struct target_file *next;
    char *source_name;
    char *target_name;
};

struct target_dir_entry {
    struct target_dir_entry *next;
    struct target_dir_entry *parent;
    struct target_dir_entry *child;
    struct target_file *head;
    char *normalized_name;
    char *case_name;
};

struct target_dir_hash {
    struct target_dir_entry *buckets[NUM_DIR_HASH_BUCKETS];
    struct target_dir_entry root;
};

struct target_dir_traversal {
    struct target_dir_entry *it;
    int i;
};

void normalize_dirname(char *filename);
void dir_hash_add_file(struct target_dir_hash *dh, const char *source, const char *target);
struct target_dir_entry *
dir_hash_create_dir(struct target_dir_hash *dh, const char *casename, const char *targetnorm);
struct target_dir_entry *dir_hash_next_dir(struct target_dir_hash *dh, struct target_dir_traversal *t);
void dir_hash_destroy(struct target_dir_hash *dh);

#endif//_REACTOS_TOOLS_CDMAKE_DIRHASH_H_
