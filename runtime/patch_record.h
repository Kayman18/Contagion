#ifndef VC_PATCH_RECORD_H
#define VC_PATCH_RECORD_H
#include "../engine/virooos.h"
#include "../engine/matrix.h"
/* Version1 stores native controls, not unsupported original Virus fields.
 * Encoding is explicit byte order, independent of C struct padding. */
#define VC_PATCH_NAME 10
#define VC_PATCH_BYTES (20u+VC_PATCH_NAME+VC_PARAMS+3u*VC_MATRIX_ROUTES)
typedef struct {char name[VC_PATCH_NAME+1];uint8_t params[VC_PARAMS];VCMatrix matrix;} VCPatchRecord;
unsigned vc_patch_schema(void);
int vc_patch_encode(uint8_t*,unsigned,const VCPatchRecord*);
int vc_patch_decode(VCPatchRecord*,const uint8_t*,unsigned);
#define VC_PATCH_SLOTS 16u
#define VC_PATCH_BANK_BYTES (20u+VC_PATCH_SLOTS*VC_PATCH_BYTES)
/* Main-loop only; storage write occurs separately after successful encoding. */
void vc_patch_bank_init(uint8_t bank[VC_PATCH_BANK_BYTES]);
int vc_patch_bank_valid(const uint8_t*,unsigned);
int vc_patch_bank_put(uint8_t*,unsigned,unsigned,const VCPatchRecord*);
int vc_patch_bank_get(VCPatchRecord*,const uint8_t*,unsigned,unsigned);
#endif
