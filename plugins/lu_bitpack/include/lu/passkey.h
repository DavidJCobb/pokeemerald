#pragma once

namespace lu {
   namespace impl::_passkey {
      union unused_parameter final {};
   }
   
   template<
      typename A,
      typename B = impl::_passkey::unused_parameter,
      typename C = impl::_passkey::unused_parameter,
      typename D = impl::_passkey::unused_parameter,
      typename E = impl::_passkey::unused_parameter
   >
   class passkey {
      friend A;
      friend B;
      friend C;
      friend D;
      friend E;
      private:
         explicit passkey() {}
   };
}