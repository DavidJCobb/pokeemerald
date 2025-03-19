#include "save_encryption.h"
#include "global.h"
#include "load_save.h" // ApplyNewEncryptionKeyToHword, ApplyNewEncryptionKeyToWord

// ApplyNewEncryptionKeyTo...
#include "berry_powder.h" // ...BerryPowder
#include "item.h" // ...BagItems_
#include "overworld.h" // ...GameStats

extern void ApplyNewEncryptionKeyToAllEncryptedData(u32 encryptionKey)
{
    ApplyNewEncryptionKeyToGameStats(encryptionKey);
    ApplyNewEncryptionKeyToBagItems_(encryptionKey);
    ApplyNewEncryptionKeyToBerryPowder(encryptionKey);
    ApplyNewEncryptionKeyToWord(&gSaveBlock1Ptr->money, encryptionKey);
    ApplyNewEncryptionKeyToHword(&gSaveBlock1Ptr->coins, encryptionKey);
}

extern void DecryptForSave(void)
{
    ApplyNewEncryptionKeyToAllEncryptedData(0);
}

extern void EncryptForSave(void)
{
    u32 backup = gSaveBlock2Ptr->encryptionKey;
    gSaveBlock2Ptr->encryptionKey = 0;
    ApplyNewEncryptionKeyToAllEncryptedData(backup);
    gSaveBlock2Ptr->encryptionKey = backup;
}