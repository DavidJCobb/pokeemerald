// present-day browsers don't format AggregateError in a usable manner, so 
// let's do it for them.
{
   function handler(event) {
      let error;
      if (event.type == "error")
         error = event.error;
      else
         error = event.reason;
      if (!(error instanceof AggregateError))
         return;
      if (error.errors.length > 0) {
         console.error(error);
         console.groupCollapsed("Constituent errors");
         for(let sub of error.errors) {
            console.error(sub);
         }
         console.groupEnd();
         //
         // NOTE: For unhandled promise rejections, `preventDefault` is broken 
         //       in Firefox and will not prevent built-in logging of the 
         //       error object.
         //
         // https://bugzilla.mozilla.org/show_bug.cgi?id=1642147
         //
         event.preventDefault();
         if (navigator.userAgent.toLowerCase().includes('firefox')) {
            console.log("NOTE: The AggregateError itself will be displayed here twice, just below this message, if Mozilla hasn't fixed some of their screwups yet.");
         }
      }
   }
   window.addEventListener("error", handler);
   window.addEventListener("unhandledrejection", handler); // errors in promises
}