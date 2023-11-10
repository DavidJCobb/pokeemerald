   u8 unknownExternalDataFields1[7]; // if actually used, may be broken up into different fields.
   u32 unknownExternalDataFields2 : 8;
   u32 currentPokeCoupons : 24; // PokéCoupons stored by Pokémon Colosseum and XD from Mt. Battle runs. Earned PokéCoupons are also added to totalEarnedPokeCoupons. Colosseum/XD caps this at 9,999,999, but will read up to 16,777,215.
   u32 gotGoldPokeCouponTitleReward : 1; // Master Ball from JP Colosseum Bonus Disc; for reaching 30,000 totalEarnedPokeCoupons
   u32 gotSilverPokeCouponTitleReward : 1; // Light Ball Pikachu from JP Colosseum Bonus Disc; for reaching 5000 totalEarnedPokeCoupons
   u32 gotBronzePokeCouponTitleReward : 1; // PP Max from JP Colosseum Bonus Disc; for reaching 2500 totalEarnedPokeCoupons
   u32 receivedAgetoCelebi : 1; // from JP Colosseum Bonus Disc
   u32 unknownExternalDataFields3 : 4;
   u32 totalEarnedPokeCoupons : 24; // Used by the JP Colosseum bonus disc. Determines PokéCoupon rank to distribute rewards. Unread in International games. Colosseum/XD caps this at 9,999,999.
   u8 unknownExternalDataFields4[5]; // if actually used, may be broken up into different fields.
