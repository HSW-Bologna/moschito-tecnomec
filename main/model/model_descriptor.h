#ifndef MODEL_DESCRIPTOR_H_INCLUDED
#define MODEL_DESCRIPTOR_H_INCLUDED

typedef struct {
    /* offset of the member from the beginning of the relative struct */
    size_t offset;
    /* size of the member */
    size_t size;
} model_member_t;

void model_descriptor_init();
model_member_t* model_descriptor_get_member(size_t model_member_idx);

#endif
