console.assert = function(condition, ...args) {
   if (condition)
      return;
   console.log(...args);
   throw new Error("Assertion failed. ", ...args); // actually halt execution
};