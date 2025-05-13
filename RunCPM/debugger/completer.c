#include "../isocline/isocline.h"

// A custom completer function.
// Use `ic_add_completion( env, replacement, display, help)` to add actual completions.
static void word_completer(ic_completion_env_t* cenv, const char* word ) 
{
  // All the words we recognise
  static const char* completions[] = {
    "base", "breakpoint", "continue", "commands", "condition", "clear",
    "delete", "disassemble", "dump", "exit", "finish", "if", "ignore", "next",
    "out", "port", "print", "read", "set", "step", "status", "tbreakpoint",
    "write", NULL };

  ic_add_completions(cenv, word, completions);
}

// A completer function is called by isocline to complete. The input parameter is the input up to the cursor.
// We use `ic_complete_word` to only consider the final token on the input. 
// (almost all user defined completers should use this)
static void completer(ic_completion_env_t* cenv, const char* input ) 
{
  // and also use our custom completer  
  ic_complete_word( cenv, input, &word_completer, NULL /* from default word boundary; whitespace or separator */ );        
  
  // ic_complete_word( cenv, input, &word_completer, &ic_char_is_idletter );        
  // ic_complete_qword( cenv, input, &word_completer, &ic_char_is_idletter  );        
}

void ic_initialize()
{
    // enable completion with a default completion function
    ic_set_default_completer(&completer, NULL);

	// enable history; use a NULL filename to not persist history to disk
	ic_set_history(NULL, -1 /* default entries (= 200) */);

}