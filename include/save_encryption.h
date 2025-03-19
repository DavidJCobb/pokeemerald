#ifndef GUARD_SAVE_ENCRYPTION_H
#define GUARD_SAVE_ENCRYPTION_H

#include "gba/types.h"

//
// Functions moved from `load_save.c` so that we can use them 
// within the new savegame code.
//

extern void ApplyNewEncryptionKeyToAllEncryptedData(u32 encryptionKey);

// Called pre-save, to save decrypted data.
extern void DecryptForSave(void);

// Called post-save, to re-encrypt data that was decrypted for save; and 
// called post-load, to encrypt data that was loaded in decrypted form.
extern void EncryptForSave(void);

#endif