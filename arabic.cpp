#include <list>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <strstream>
#include "arabic.h"

// Input functions

inline
void aasaan_either_or(std::istream& in, std::list<arabic_letter>& out,
		      arabic_token_t first, arabic_token_t second)
{
  char c;
  in.get(c);
  if (in.eof()) {
    out.push_back(arabic_letter(first, TF_CONSONANT));
  } else {
    switch (c) {
    case 'h':
      out.push_back(arabic_letter(second, TF_CONSONANT));
      break;

    default:
      in.unget();
    case '_':
      out.push_back(arabic_letter(first, TF_CONSONANT));
      break;
    }
  }
}

/* Turn a stream of Aasaan transliterated text into unambiguous
   tokens, which can be used to render any form of output
   encoding. */

#ifdef MODE_STACK
static std::vector<arabic_mode_t> mode_stack;
#endif

#define push(out, tok, flags) (out).push_back(arabic_letter((tok), (flags)))

bool parse_aasaan(std::istream& in, std::list<arabic_letter>& out,
		  arabic_mode_t mode, bool only_one = false)
{
  char c;
  unsigned int start_size = out.size();
  bool capitalize_next = false;

  in.get(c);
  while (! in.eof()) {
    arabic_letter& last = out.back();

    switch (c) {
    case 'a': {
      bool parsed = false;

      in.get(c);
      if (! in.eof()) {
	switch (c) {
	case 'a':
	  if (arabic_is_letter(last))
	    last.flags |= TF_FATHA;
	  else
	    push(out, TOK_ALIF,
		 TF_CONSONANT | TF_CARRIER | TF_FATHA);

	  parsed = true;
	  push(out, TOK_ALIF, TF_VOWEL);
	  break;

	case 'l':
	  if (! arabic_is_letter(last))
	    {
	      in.get(c);
	      if (! in.eof()) {
		if (c == '-') {
		  parsed = true;
		  push(out, TOK_PREFIX_AL, TF_NO_FLAGS);
		  break;
		}
		in.unget();
	      }
	    }
	  // else fall through...

	default: {
	  in.unget();	// only parse the first 'a'
	  if (c == 'l')
	    break;

	  // This rather complicated code allows for two
	  // different styles of specifying sun letters:
	  //
	  // Style 1: al-rra.hman
	  // Style 2: ar-ra.hman
	  //
	  // The first style is easy to parse, and is
	  // handled as AL + R/SHADDA ...; but the second
	  // requires foreward lookahead, which is done
	  // below.

	  std::streampos before = in.tellg();

	  if (arabic_is_letter(last) ||
	      ! parse_aasaan(in, out, mode, true)) {
	    break;
	  }
		    
	  arabic_letter& prev = out.back();
	  arabic_letter copy = prev;

	  // If the parse below succeeds in finding another
	  // character, it means we did not see a doubled
	  // letter (TF_SHADDA).

	  in.get(c);
	  if (in.eof() || c != '-')
	    {
	      out.pop_back();
	      in.seekg(before);
	      break;
	    }

	  if (parse_aasaan(in, out, mode, true))
	    {
	      out.pop_back();
	      out.pop_back();
	      in.seekg(before);
	      break;
	    }

	  // We still have to check, since we might have
	  // ended the input at "ar-", in which case we
	  // would still get to this point.

	  bool match = prev.flags & TF_SHADDA;

	  out.pop_back();

	  if (match) {
	    parsed = true;
	    push(out, TOK_PREFIX_AL, TF_SUN_LETTER);
	    copy.flags |= TF_SHADDA;
	    out.push_back(copy);
	  }
	  break;
	}
	}
      }

      if (! parsed) {
	if (arabic_is_letter(last))
	  last.flags |= TF_FATHA;
	else
	  push(out, TOK_ALIF,
	       TF_CONSONANT | TF_CARRIER | TF_FATHA);
      }
      break;
    }

    case 'i': {
      bool parsed = false;

      in.get(c);
      if (! in.eof()) {
	switch (c) {
	case 'i':
	  if (arabic_is_letter(last))
	    last.flags |= TF_KASRA;
	  else
	    push(out, TOK_ALIF,
		 TF_CONSONANT | TF_CARRIER | TF_KASRA);

	  parsed = true;
	  push(out, TOK_YIH, TF_VOWEL);
	  break;

	default:
	  in.unget();	// only parse the first 'i'
	  break;
	}
      }

      if (! parsed) {
	if (arabic_is_letter(last))
	  last.flags |= TF_KASRA;
	else
	  push(out, TOK_ALIF,
	       TF_CONSONANT | TF_CARRIER | TF_KASRA);
      }
      break;
    }

    case 'u': {
      bool parsed = false;

      in.get(c);
      if (! in.eof()) {
	if (c == 'u' || c == 'w') {
	  parsed = true;

	  if (arabic_is_letter(last))
	    last.flags |= TF_DHAMMA;
	  else
	    push(out, TOK_ALIF,
		 TF_CONSONANT | TF_CARRIER | TF_DHAMMA);

	  push(out, TOK_WAAW,
	       c == 'u' ? TF_VOWEL : TF_CONSONANT | TF_DIPHTHONG);
	} else {
	  in.unget();	// only parse the first 'u'
	}
      }

      if (! parsed) {
	if (arabic_is_letter(last))
	  last.flags |= TF_DHAMMA;
	else
	  push(out, TOK_ALIF,
	       TF_CONSONANT | TF_CARRIER | TF_DHAMMA);
      }
      break;
    }

    case 'o':
      if (arabic_is_letter(last))
	last.flags |= TF_DHAMMA;
      push(out, TOK_WAAW, TF_VOWEL | TF_SILENT);
      break;

    case 'e':
      in.get(c);
      if (! in.eof()) {
	if (c == 'y') {
	  if (arabic_is_letter(last))
	    last.flags |= TF_KASRA;
	  else
	    push(out, TOK_ALIF,
		 TF_CONSONANT | TF_CARRIER | TF_KASRA);
	  push(out, TOK_YIH, TF_CONSONANT | TF_DIPHTHONG);
	} else {
	  in.unget();
	}
      }
      break;

    case 'b':
      push(out, TOK_BIH, TF_CONSONANT);
      break;

    case 'p':
      push(out, TOK_PIH, TF_CONSONANT);
      break;

    case 't':
      aasaan_either_or(in, out, TOK_TIH, TOK_THIH);
      break;

    case 'j':
      push(out, TOK_JIIM, TF_CONSONANT);
      break;

    case 'c':
      in.get(c);
      if (! in.eof()) {
	if (c == 'h') {
	  push(out, TOK_CHIH, TF_CONSONANT);
	} else {
	  in.unget();
	}
      }
      break;

    case 'h': {
      // Foreward lookahead is needed here, to determine if
      // this is a final hih or not; further, this only applies
      // to the Persian language.
      //
      // If it is a final heh, and there is no long vowel or
      // diphthong before it, then in Persian it is a "silent
      // final heh".  This influences the writing of izafih,
      // the indefinite enclitic, and suffix copula verbs or
      // possession articles.  In cases where silent heh would
      // be chosen, use H to force non-silent heh.

      unsigned long flags = TF_CONSONANT;

      in.get(c);

      if (mode == MODE_PERSIAN &&
	  last.flags & TF_CONSONANT &&
	  ! (last.flags & TF_DIPHTHONG) &&
	  (in.eof() || c == '-' || isspace(c)))
	{
	  flags |= TF_SILENT;
	}
      in.unget();

      push(out, TOK_HIH, flags);
      break;
    }

    case 'H':
      push(out, TOK_HIH, TF_CONSONANT);
      break;

    case 'k':
      aasaan_either_or(in, out, TOK_KAAF, TOK_KHIH);
      break;

    case 'd':
      aasaan_either_or(in, out, TOK_DAAL, TOK_DHAAL);
      break;

    case 's':
      aasaan_either_or(in, out, TOK_SIIN, TOK_SHIIN);
      break;

    case 'r':
      push(out, TOK_RIH, TF_CONSONANT);
      break;

    case 'z':
      aasaan_either_or(in, out, TOK_ZIH, TOK_ZHIH);
      break;

    case '`':
      in.get(c);
      if (! in.eof()) {
	if (c == '`') {
	  push(out, TOK_LEFT_QUOTE, TF_NO_FLAGS);
	  break;
	} else {
	  in.unget();
	}
      }
      push(out, TOK_AYN, TF_CONSONANT);
      break;

    case 'g':
      aasaan_either_or(in, out, TOK_GAAF, TOK_GHAYN);
      break;

    case 'f':
      push(out, TOK_FIH, TF_CONSONANT);
      break;

    case 'q':
      push(out, TOK_QAAF, TF_CONSONANT);
      break;

    case 'l':
      push(out, TOK_LAAM, TF_CONSONANT);
      break;

    case 'm':
      push(out, TOK_MIIM, TF_CONSONANT);
      break;

    case 'n':
      push(out, TOK_NUUN, TF_CONSONANT);
      break;

    case 'w':
    case 'v':
      push(out, TOK_WAAW, TF_CONSONANT);
      break;

    case 'y':
      push(out, TOK_YIH, TF_CONSONANT);
      break;

    case '.':
      in.get(c);
      if (! in.eof()) {
	switch (c) {
	case 'h':
	  push(out, TOK_HIH_HUTII, TF_CONSONANT);
	  break;

	case 's':
	  push(out, TOK_SAAD, TF_CONSONANT);
	  break;

	case 'd':
	  push(out, TOK_THAAD, TF_CONSONANT);
	  break;

	case 't':
	  push(out, TOK_TAYN, TF_CONSONANT);
	  break;

	case 'z':
	  push(out, TOK_DTHAYN, TF_CONSONANT);
	  break;

	default:
	  in.unget();
	  push(out, TOK_PERIOD, TF_NO_FLAGS);
	  break;
	}
      }
      break;

    case 'N':
      if (arabic_is_letter(last))
	last.flags |= TF_TANWEEN;
      break;

    case 'T':
      push(out, TOK_TIH_MARBUTA, TF_CONSONANT);
      break;

    case 'Y':
      push(out, TOK_ALIF_MAQSURA, TF_VOWEL);
      break;

    case '\'':
      in.get(c);
      if (! in.eof()) {
	if (c == '\'') {
	  push(out, TOK_RIGHT_QUOTE, TF_NO_FLAGS);
	  break;
	} else {
	  in.unget();
	}
      }
      push(out, TOK_HAMZA, TF_CONSONANT);
      break;

#ifdef MODE_STACK
    case 'A':
      in.get(c);
      if (! in.eof()) {
	if (c == '/') {
	  mode_stack.push_back(mode = MODE_ARABIC);
	  push(out, TOK_PUSH_MODE, mode);
	} else {
	  in.unget();
	}
      }
      break;
#endif // MODE_STACK

    case 'U':
      in.get(c);
      if (! in.eof()) {
	if (c == 'A') {
	  // UA must occur at the end of a word (this is
	  // not enforced, however), and produces a vowel
	  // waaw with a final silent alif

	  assert(arabic_is_letter(last));
	  last.flags |= TF_DHAMMA;
	  push(out, TOK_WAAW, TF_VOWEL | TF_SILENT_ALIF);
	} else {
	  in.unget();
	}
      }
      break;

#ifdef MODE_STACK
    case 'P':
      in.get(c);
      if (! in.eof()) {
	if (c == '/') {
	  mode_stack.push_back(mode = MODE_PERSIAN);
	  push(out, TOK_PUSH_MODE, mode);
	} else {
	  in.unget();
	}
      }
      break;

    case '/':
      in.get(c);
      if (! in.eof()) {
	if (c == 'A') {
	  if (! mode_stack.empty()) {
	    mode = mode_stack.back();
	    mode_stack.pop_back();
	  }
	  push(out, TOK_POP_MODE, mode);
	} else if (c == 'P') {
	  if (! mode_stack.empty()) {
	    mode = mode_stack.back();
	    mode_stack.pop_back();
	  }
	  push(out, TOK_POP_MODE, mode);
	} else {
	  in.unget();
	}
      }
      break;
#endif // MODE_STACK

    case '^':
      capitalize_next = true;
      goto NEXT;

    case ',':
      push(out, TOK_COMMA, TF_NO_FLAGS);
      break;

    case ';':
      push(out, TOK_SEMICOLON, TF_NO_FLAGS);
      break;

    case ':':
      push(out, TOK_COLON, TF_NO_FLAGS);
      break;

    case '!':
      push(out, TOK_EXCLAM, TF_NO_FLAGS);
      break;

    case '?':
      push(out, TOK_QUERY, TF_NO_FLAGS);
      break;

    case '~':
      parse_aasaan(in, out, mode, true);
      out.back().flags |= TF_SHADDA;
      break;

    case '_':
      in.get(c);
      if (! in.eof()) {
	if (c == 'a') {
	  if (arabic_is_letter(last))
	    last.flags |= TF_DEFECTIVE_ALIF;
	} else {
	  in.unget();
	}
      }
      break;

    case '-':
      in.get(c);
      if (! in.eof()) {
	if (! arabic_is_letter(last)) {
	  in.unget();
	}
	else if (c == 'i') {
	  in.get(c);
	  if (! in.eof()) {
	    if (isspace(c)) {
	      last.flags |= TF_IZAAFIH;
	      in.unget();	// let it be separate words
	      break;
	    }
	    else if (c == 'i') {
	      in.get(c);
	      if (! in.eof()) {
		if (isspace(c)) {
		  push(out, TOK_SUFFIX_II, TF_NO_FLAGS);
		  in.unget();
		  break;
		}
		in.unget();
	      }
	    }
	    in.unget();
	  }
	}
	else if (c == 'r' || c == 'h') {
	  int f = c;
	  in.get(c);
	  if (! in.eof()) {
	    if ('a') {
	      in.get(c);
	      if (! in.eof()) {
		if (c == 'a') {
		  in.get(c);
		  if (! in.eof()) {
		    if (isspace(c)) {
		      push(out, f == 'r' ? TOK_SUFFIX_RAA :
			   TOK_SUFFIX_HAA, TF_NO_FLAGS);
		      in.unget();
		      break;
		    }
		    in.unget();
		  }
		}
		in.unget();
	      }
	    }
	    in.unget();
	  }
	} else {
	  in.unget();
	}
      }
      push(out, TOK_SPACER, TF_NO_FLAGS);
      break;

    default:
      push(out, TOK_UNKNOWN, (unsigned long) c);
      break;

    case '\t':
    case ' ':
    case '\n': {
      // Gobble up multiple whitespace characters; they
      // collapse into TOK_SPACE, unless there are two returns,
      // in which case it is a paragraph separator

      int ret = 0;
      while (in.get(c) && ! in.eof() && isspace(c)) {
	if (c == '\n') ret++;
      }
      if (! in.eof())
	in.unget();

      if (ret > 1)
	push(out, TOK_PARAGRAPH, TF_NO_FLAGS);
      else
	push(out, TOK_SPACE, TF_NO_FLAGS);
      break;
    }
    }

    if (! (last.flags & TF_SHADDA) && arabic_is_sukun(last) &&
	&last != &out.back() && last == out.back())
      {
	last.flags |= TF_SHADDA;
	out.pop_back();

	// Check if this is a sun letter
	std::list<arabic_letter>::iterator i = out.end();
	i--; assert(*i == out.back());
	i--;

	if (i->token == TOK_PREFIX_AL)
	  i->flags |= TF_SUN_LETTER;
      }

    if (capitalize_next) {
      out.back().flags |= TF_CAPITALIZE;
      capitalize_next = false;
    }

  NEXT:
    if (only_one && ! capitalize_next)
      break;
    else if (! in.eof())
      in.get(c);
  }

  return start_size != out.size();
}

bool parse_talattof(std::istream& in, std::list<arabic_letter>& out,
		    arabic_mode_t mode, bool only_one)
{
  char c;
  unsigned int start_size = out.size();

  in.get(c);
  while (! in.eof()) {
    arabic_letter& last = out.back();

    switch (c) {
    case '\015': push(out, TOK_PARAGRAPH, TF_NO_FLAGS); break;

    case '\040': push(out, TOK_SPACE, TF_NO_FLAGS); break;
    case '\240': push(out, TOK_SPACE, TF_NO_FLAGS); break;
    case '\256': push(out, TOK_PERIOD, TF_NO_FLAGS); break;

#if 0
    case '\260': push(out, TOK_ZERO, TF_NO_FLAGS); break;
    case '\261': push(out, TOK_ONE, TF_NO_FLAGS); break;
    case '\262': push(out, TOK_TWO, TF_NO_FLAGS); break;
    case '\263': push(out, TOK_THREE, TF_NO_FLAGS); break;
    case '\264': push(out, TOK_FOUR, TF_NO_FLAGS); break;
    case '\265': push(out, TOK_FIVE, TF_NO_FLAGS); break;
    case '\266': push(out, TOK_SIX, TF_NO_FLAGS); break;
    case '\267': push(out, TOK_SEVEN, TF_NO_FLAGS); break;
    case '\270': push(out, TOK_EIGHT, TF_NO_FLAGS); break;
    case '\271': push(out, TOK_NINE, TF_NO_FLAGS); break;
#endif

    case '\302':
      push(out, TOK_ALIF, TF_VOWEL | TF_FATHA | TF_BAA_KULAA);
      break;
	    
    case '\306':
      push(out, TOK_HAMZA, TF_CONSONANT | TF_KASRA);

    case '\307':
      if (arabic_is_letter(last))
	push(out, TOK_ALIF, TF_VOWEL | TF_FATHA);
      else
	push(out, TOK_ALIF, TF_CONSONANT | TF_FATHA);
      break;

    case '\310': push(out, TOK_BIH, TF_CONSONANT); break;
    case '\312': push(out, TOK_TIH, TF_CONSONANT); break;
    case '\313': push(out, TOK_THIH, TF_CONSONANT); break;
    case '\314': push(out, TOK_JIIM, TF_CONSONANT); break;
    case '\315': push(out, TOK_HIH_HUTII, TF_CONSONANT); break;
    case '\316': push(out, TOK_KHIH, TF_CONSONANT); break;
    case '\317': push(out, TOK_DAAL, TF_CONSONANT); break;
    case '\320': push(out, TOK_DHAAL, TF_CONSONANT); break;
    case '\321': push(out, TOK_RIH, TF_CONSONANT); break;
    case '\322': push(out, TOK_ZIH, TF_CONSONANT); break;
    case '\323': push(out, TOK_SIIN, TF_CONSONANT); break;
    case '\324': push(out, TOK_SHIIN, TF_CONSONANT); break;
    case '\325': push(out, TOK_SAAD, TF_CONSONANT); break;
    case '\326': push(out, TOK_THAAD, TF_CONSONANT); break;
    case '\327': push(out, TOK_TAYN, TF_CONSONANT); break;
    case '\330': push(out, TOK_DTHAYN, TF_CONSONANT); break;
    case '\331': push(out, TOK_AYN, TF_CONSONANT); break;
    case '\332': push(out, TOK_GHAYN, TF_CONSONANT); break;
    case '\341': push(out, TOK_FIH, TF_CONSONANT); break;
    case '\342': push(out, TOK_QAAF, TF_CONSONANT); break;
    case '\343': push(out, TOK_KAAF, TF_CONSONANT); break;
    case '\344': push(out, TOK_LAAM, TF_CONSONANT); break;
    case '\345': push(out, TOK_MIIM, TF_CONSONANT); break;
    case '\346': push(out, TOK_NUUN, TF_CONSONANT); break;
    case '\347': push(out, TOK_HIH, TF_CONSONANT); break;
    case '\350': push(out, TOK_WAAW, TF_CONSONANT); break;
    case '\351': push(out, TOK_YIH, TF_CONSONANT); break; // final
    case '\352': push(out, TOK_YIH, TF_CONSONANT); break;

    case '\354': push(out, TOK_HAMZA, TF_CONSONANT); break;

    case '\353':
      if (arabic_is_letter(last)) {
	if (last.token == TOK_ALIF) {
	  out.pop_back();
	  out.back().flags |= TF_FATHA | TF_TANWEEN;
	} else {
	  last.flags |= TF_TANWEEN;
	}
      }
      break;

    case '\360':
      if (arabic_is_letter(last))
	last.flags |= TF_IZAAFIH | TF_EXPLICIT;
      break;

    case '\363': push(out, TOK_PIH, TF_CONSONANT); break;
    case '\370': push(out, TOK_GAAF, TF_CONSONANT); break;
    case '\376': push(out, TOK_ZHIH, TF_CONSONANT); break;

    default: push(out, TOK_EXCLAM, TF_NO_FLAGS); break;
    }

    if (! in.eof())
      in.get(c);
  }

  return start_size != out.size();
}

// Output functions

bool output_aasaan_letter(std::list<arabic_letter>::iterator letter,
			  std::list<arabic_letter>::iterator end,
			  std::ostream& out, arabic_mode_t mode,
			  bool quiet_shadda)
{
  switch (letter->token) {
  case TOK_ALIF:
    if (letter->flags & TF_VOWEL)
      out << 'a';
    break;

  case TOK_BIH:
    out << 'b';
    break;

  case TOK_TIH:
    out << 't';
    break;

  case TOK_THIH:
    out << "th";
    break;

  case TOK_JIIM:
    out << 'j';
    break;

  case TOK_HIH_HUTII:
    out << ".h";
    break;

  case TOK_KHIH:
    out << "kh";
    break;

  case TOK_SIIN:
    out << 's';
    break;

  case TOK_SHIIN:
    out << "sh";
    break;

  case TOK_DAAL:
    out << 'd';
    break;

  case TOK_DHAAL:
    out << "dh";
    break;

  case TOK_RIH:
    out << 'r';
    break;

  case TOK_ZIH:
    out << 'z';
    break;

  case TOK_SAAD:
    out << ".s";
    break;

  case TOK_THAAD:
    out << ".d";
    break;

  case TOK_TAYN:
    out << ".t";
    break;

  case TOK_DTHAYN:
    out << ".z";
    break;

  case TOK_AYN:
    out << '`';
    break;

  case TOK_GHAYN:
    out << "gh";
    break;

  case TOK_FIH:
    out << 'f';
    break;

  case TOK_QAAF:
    out << 'q';
    break;

  case TOK_KAAF:
    out << 'k';
    break;

  case TOK_LAAM:
    out << 'l';
    break;

  case TOK_MIIM:
    out << 'm';
    break;

  case TOK_NUUN:
    out << 'n';
    break;

  case TOK_WAAW:
    if (letter->flags & TF_DIPHTHONG) {
      out << 'w';
    }
    else if (letter->flags & TF_CONSONANT) {
      if (mode == MODE_ARABIC)
	out << 'w';
      else
	out << 'v';
    }
    else if (letter->flags & TF_VOWEL) {
      if (letter->flags & TF_SILENT)
	out << 'o';
      else if (letter->flags & TF_SILENT_ALIF)
	out << "UA";
      else
	out << 'u';
    }
    break;

  case TOK_YIH:
    if (letter->flags & (TF_CONSONANT | TF_DIPHTHONG))
      out << 'y';
    else if (letter->flags & TF_VOWEL) {
      out << 'i';
    }
    break;

  case TOK_ALIF_MAQSURA:
    out << 'Y';
    break;

  case TOK_HIH:
    if (letter->flags & TF_SILENT || mode == MODE_ARABIC) {
      out << 'h';
    } else {
      std::list<arabic_letter>::iterator prev = letter; prev--;
      std::list<arabic_letter>::iterator next = letter; next++;

      if (arabic_is_sukun(*letter) &&
	  (next == end || ! arabic_is_letter(*next)) &&
	  ! (prev->flags & (TF_VOWEL | TF_DIPHTHONG |
			    TF_DEFECTIVE_ALIF)))
	{
	  out << 'H';
	} else {
	out << 'h';
      }
    }
    break;

  case TOK_TIH_MARBUTA:
    out << 'T';
    break;

  case TOK_HAMZA:
    out << '\'';
    break;


    // Persian letters

  case TOK_PIH:
    out << 'p';
    break;

  case TOK_CHIH:
    out << "ch";
    break;

  case TOK_ZHIH:
    out << "zh";
    break;

  case TOK_GAAF:
    out << 'g';
    break;

  default:
    return false;
  }

  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  std::list<arabic_letter>::iterator next = letter;
  next++;

  if (letter->flags & TF_FATHA) {
    out << 'a';
  }
  else if (letter->flags & TF_KASRA) {
    if (next != end && next->flags & TF_DIPHTHONG)
      out << 'e';
    else
      out << 'i';
  }
  else if (letter->flags & TF_DHAMMA) {
    if (next == end || ! (next->flags & TF_SILENT))
      out << 'u';
  }
  else if (letter->flags & TF_DEFECTIVE_ALIF) {
    out << "_a";
  }
  else if (letter->flags & TF_IZAAFIH) {
    out << "-i";
  }
  else {
    // s t d k g z, followed by a natural h, must output
    // a _ between the letter to prevent incorrect
    // interpretation

    if (next != end &&
	(letter->token == TOK_SIIN ||
	 letter->token == TOK_TIH ||
	 letter->token == TOK_DAAL ||
	 letter->token == TOK_KAAF ||
	 letter->token == TOK_GAAF ||
	 letter->token == TOK_ZIH) &&
	next->flags & TF_CONSONANT &&
	next->token == TOK_HIH)
      {
	out << '_';
      }
  }

  if (letter->flags & TF_TANWEEN)
    out << 'N';
    
  return true;
}

void output_aasaan(std::list<arabic_letter>& in, std::ostream& out,
		   arabic_mode_t mode)
{
  std::list<arabic_letter>::iterator letter = in.begin();

  while (letter != in.end()) {
    if (letter->flags & TF_CAPITALIZE)
      out << '^';
    if (letter->token == TOK_AYN && letter->flags & TF_SHADDA) {
      out << '~';
      output_aasaan_letter(letter, in.end(), out, mode, false);
    }
    else if (output_aasaan_letter(letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
	output_aasaan_letter(letter, in.end(), out, mode, false);
    }
    else {
      switch (letter->token) {
      case TOK_PREFIX_AL:
	if (! (letter->flags & TF_SUN_LETTER)) {
	  out << "al-";
	  break;
	}
	letter++;

	out << 'a';
	output_aasaan_letter(letter, in.end(), out, mode, true);
	out << '-';

	// when laam is sun, it must be output: al-ll
	if (letter->token == TOK_LAAM)
	  output_aasaan_letter(letter, in.end(), out, mode, true);
	output_aasaan_letter(letter, in.end(), out, mode, false);

	break;

      case TOK_PREFIX_BI:
	out << "bi-";
	break;

      case TOK_PREFIX_MII:
	out << "mii-";
	break;

      case TOK_SUFFIX_RAA:
	out << "-raa";
	break;

      case TOK_SUFFIX_HAA:
	out << "-haa";
	break;

      case TOK_SUFFIX_II:
	out << "-ii";
	break;


      case TOK_SPACE:
	out << ' ';
	break;

      case TOK_PERIOD:
	out << '.';
	break;

      case TOK_COMMA:
	out << ',';
	break;

      case TOK_SEMICOLON:
	out << ';';
	break;

      case TOK_COLON:
	out << ':';
	break;

      case TOK_EXCLAM:
	out << '!';
	break;

      case TOK_QUERY:
	out << '?';
	break;

      case TOK_PARAGRAPH:
	out << std::endl
	    << std::endl;
	break;


      case TOK_SPACER:
	out << '-';
	break;


      case TOK_LEFT_QUOTE:
	out << "``";
	break;

      case TOK_RIGHT_QUOTE:
	out << "''";
	break;


#ifdef MODE_STACK
      case TOK_PUSH_MODE:
	if (letter->flags == (unsigned long) MODE_ARABIC) {
	  mode = MODE_ARABIC;
	  out << "A/";
	}
	else if (letter->flags == (unsigned long) MODE_PERSIAN) {
	  mode = MODE_PERSIAN;
	  out << "P/";
	}
	break;

      case TOK_POP_MODE:
	if (mode == MODE_ARABIC)
	  out << "/A";
	else if (mode == MODE_PERSIAN)
	  out << "/P";

	mode = (arabic_mode_t) letter->flags;
	break;
#endif // MODE_STACK


      case TOK_UNKNOWN:
	out << (char)letter->flags;
	break;

      default:
	std::cerr << "output_aasaan: unhandled token "
		  << letter->token << std::endl;
	break;
      }
    }

    letter++;
  }
}

bool output_arabtex_letter(std::list<arabic_letter>::iterator letter,
			   std::list<arabic_letter>::iterator end,
			   std::ostream& out, arabic_mode_t mode,
			   bool quiet_shadda)
{
  switch (letter->token) {
  case TOK_ALIF:
    if (letter->flags & TF_VOWEL)
      out << 'a';
    break;

  case TOK_BIH:
    out << 'b';
    break;

  case TOK_TIH:
    out << 't';
    break;

  case TOK_THIH:
    out << "_t";
    break;

  case TOK_JIIM:
    out << "^g";
    break;

  case TOK_HIH_HUTII:
    out << ".h";
    break;

  case TOK_KHIH:
    out << "_h";
    break;

  case TOK_SIIN:
    out << 's';
    break;

  case TOK_SHIIN:
    out << "^s";
    break;

  case TOK_DAAL:
    out << 'd';
    break;

  case TOK_DHAAL:
    out << "_d";
    break;

  case TOK_RIH:
    out << 'r';
    break;

  case TOK_ZIH:
    out << 'z';
    break;

  case TOK_SAAD:
    out << ".s";
    break;

  case TOK_THAAD:
    out << ".d";
    break;

  case TOK_TAYN:
    out << ".t";
    break;

  case TOK_DTHAYN:
    out << ".z";
    break;

  case TOK_AYN:
    out << '`';
    break;

  case TOK_GHAYN:
    out << ".g";
    break;

  case TOK_FIH:
    out << 'f';
    break;

  case TOK_QAAF:
    out << 'q';
    break;

  case TOK_KAAF:
    out << 'k';
    break;

  case TOK_LAAM:
    out << 'l';
    break;

  case TOK_MIIM:
    out << 'm';
    break;

  case TOK_NUUN:
    out << 'n';
    break;

  case TOK_WAAW:
    // If this waaw is followed by a silent alif, then the
    // encoding has already been done during the handling of the
    // preceding letter's DHAMMA, below
	
    if (! (letter->flags & TF_SILENT_ALIF))
      out << 'w';
    break;

  case TOK_YIH:
    out << 'y';
    break;

  case TOK_ALIF_MAQSURA:
    out << "Y_a";
    break;

  case TOK_HIH:
    if (letter->flags & TF_SILENT)
      out << 'H';
    else
      out << 'h';
    break;

  case TOK_TIH_MARBUTA:
    out << 'T';
    break;

  case TOK_HAMZA:
    out << '\'';
    break;


    // Persian letters

  case TOK_PIH:
    out << 'p';
    break;

  case TOK_CHIH:
    out << "^c";
    break;

  case TOK_ZHIH:
    out << "^z";
    break;

  case TOK_GAAF:
    out << 'g';
    break;

  default:
    return false;
  }

  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  std::list<arabic_letter>::iterator next = letter;
  next++;

  if (letter->flags & TF_TANWEEN)
    out << '"';

  if (letter->flags & TF_FATHA) {
    out << 'a';
  }
  else if (letter->flags & TF_KASRA) {
    if (next != end && next->flags & TF_DIPHTHONG)
      out << 'e';
    else
      out << 'i';
  }
  else if (letter->flags & TF_DHAMMA) {
    if (next != end && next->flags & TF_DIPHTHONG)
      out << 'o';
    else if (next->flags & TF_SILENT_ALIF)
      out << "UA";
    else
      out << 'u';
  }
  else if (letter->flags & TF_DEFECTIVE_ALIF) {
    out << "_a";
  }
  else if (letter->flags & TF_IZAAFIH) {
    if (letter->flags & TF_VOWEL)
      out << "y-i";
    else
      out << "-i";
  }

  if (letter->flags & TF_TANWEEN)
    out << "N";

  return true;
}

void output_arabtex(std::list<arabic_letter>& in, std::ostream& out,
		    arabic_mode_t mode)
{
  std::list<arabic_letter>::iterator letter = in.begin();

  while (letter != in.end()) {
    if (output_arabtex_letter(letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
	output_arabtex_letter(letter, in.end(), out, mode, false);
    }
    else {
      switch (letter->token) {
      case TOK_PREFIX_AL:
	out << "al-";
	break;

      case TOK_PREFIX_BI:
	out << "bi-";
	break;

      case TOK_PREFIX_MII:
	out << "mI\\hspace{0.4ex}";
	break;

      case TOK_SUFFIX_RAA:
	out << "\\hspace{0.4ex}raa";
	break;

      case TOK_SUFFIX_HAA:
	out << "-haa";
	break;

      case TOK_SUFFIX_II:
	out << "-I";
	break;


      case TOK_SPACE:
	out << ' ';
	break;

      case TOK_PERIOD:
	out << '.';
	break;

      case TOK_COMMA:
	out << ',';
	break;

      case TOK_SEMICOLON:
	out << ';';
	break;

      case TOK_COLON:
	out << ':';
	break;

      case TOK_EXCLAM:
	out << '!';
	break;

      case TOK_QUERY:
	out << '?';
	break;

      case TOK_PARAGRAPH:
	out << std::endl << std::endl;
	break;


      case TOK_SPACER:
	out << '-';
	break;


      case TOK_LEFT_QUOTE:
	out << "\\lq ";
	break;

      case TOK_RIGHT_QUOTE:
	out << "\\rq ";
	break;


#ifdef MODE_STACK
      case TOK_PUSH_MODE:
      case TOK_POP_MODE:
	if (letter->flags == (unsigned long) MODE_ARABIC)
	  out << "\\setarab \\newtanwin";
	else if (letter->flags == (unsigned long) MODE_PERSIAN)
	  out << "\\setfarsi \\newtanwin";
	break;
#endif // MODE_STACK


      case TOK_UNKNOWN:
	out << (char)letter->flags;
	break;

      default:
	std::cerr << "output_arabtex: unhandled token "
		  << letter->token << std::endl;
	break;
      }
    }

    letter++;
  }
}

bool output_unicode_letter(std::list<arabic_letter>::iterator letter,
			   std::list<arabic_letter>::iterator end,
			   std::ostream& out, arabic_mode_t mode,
			   bool quiet_shadda)
{
  switch (letter->token) {
  case TOK_ALIF:
    if (letter->flags & TF_VOWEL)
      out << "&#1575;";
    break;

  case TOK_BIH:
    out << "&#1576;";
    break;

  case TOK_TIH:
    out << "&#1578;";
    break;

  case TOK_THIH:
    out << "&#1579;";
    break;

  case TOK_JIIM:
    out << "&#1580;";
    break;

  case TOK_HIH_HUTII:
    out << "&#1581;";
    break;

  case TOK_KHIH:
    out << "&#1582;";
    break;

  case TOK_SIIN:
    out << "&#1587;";
    break;

  case TOK_SHIIN:
    out << "&#1588;";
    break;

  case TOK_DAAL:
    out << "&#1583;";
    break;

  case TOK_DHAAL:
    out << "&#1584;";
    break;

  case TOK_RIH:
    out << "&#1585;";
    break;

  case TOK_ZIH:
    out << "&#1586;";
    break;

  case TOK_SAAD:
    out << "&#1589;";
    break;

  case TOK_THAAD:
    out << "&#1590;";
    break;

  case TOK_TAYN:
    out << "&#1591;";
    break;

  case TOK_DTHAYN:
    out << "&#1592;";
    break;

  case TOK_AYN:
    out << "&#1593;";
    break;

  case TOK_GHAYN:
    out << "&#1594;";
    break;

  case TOK_FIH:
    out << "&#1601;";
    break;

  case TOK_QAAF:
    out << "&#1602;";
    break;

  case TOK_KAAF:
    out << "&#1705;";
    break;

  case TOK_LAAM:
    out << "&#1604;";
    break;

  case TOK_MIIM:
    out << "&#1605;";
    break;

  case TOK_NUUN:
    out << "&#1606;";
    break;

  case TOK_WAAW:
    // If this waaw is followed by a silent alif, then the
    // encoding has already been done during the handling of the
    // preceding letter's DHAMMA, below
	
    if (! (letter->flags & TF_SILENT_ALIF))
      out << "&#1608;";
    break;

  case TOK_YIH:
    out << "&#1610;";
    break;

  case TOK_ALIF_MAQSURA:
    out << "&#1609;";
    break;

  case TOK_HIH:
    if (letter->flags & TF_SILENT)
      out << "&#1607;";
    else
      out << "&#1607;";
    break;

  case TOK_TIH_MARBUTA:
    out << "&#1577;";
    break;

  case TOK_HAMZA:
    out << "";
    break;


    // Persian letters

  case TOK_PIH:
    out << "&#1662;";
    break;

  case TOK_CHIH:
    out << "&#1670;";
    break;

  case TOK_ZHIH:
    out << "&#1688;";
    break;

  case TOK_GAAF:
    out << "&#1711;";
    break;

  default:
    return false;
  }

  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  std::list<arabic_letter>::iterator next = letter;
  next++;

#if 0
  if (letter->flags & TF_TANWEEN)
    out << '"';

  if (letter->flags & TF_FATHA) {
    out << 'a';
  }
  else if (letter->flags & TF_KASRA) {
    if (next != end && next->flags & TF_DIPHTHONG)
      out << 'e';
    else
      out << 'i';
  }
  else if (letter->flags & TF_DHAMMA) {
    if (next != end && next->flags & TF_DIPHTHONG)
      out << 'o';
    else if (next->flags & TF_SILENT_ALIF)
      out << "UA";
    else
      out << 'u';
  }
  else if (letter->flags & TF_DEFECTIVE_ALIF) {
    out << "_a";
  }
  else if (letter->flags & TF_IZAAFIH) {
    if (letter->flags & TF_VOWEL)
      out << "y-i";
    else
      out << "-i";
  }

  if (letter->flags & TF_TANWEEN)
    out << "N";
#endif

  return true;
}

void output_unicode(std::list<arabic_letter>& in, std::ostream& out,
		    arabic_mode_t mode)
{
  std::list<arabic_letter>::iterator letter = in.begin();

  while (letter != in.end()) {
    if (output_unicode_letter(letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
	output_unicode_letter(letter, in.end(), out, mode, false);
    }
    else {
      switch (letter->token) {
      case TOK_PREFIX_AL:
	out << "&#1575;&#1604;";
	break;

      case TOK_PREFIX_BI:
	out << "&#1576;";
	break;

      case TOK_PREFIX_MII:
	out << "&#1605;&#1610; ";
	break;

      case TOK_SUFFIX_RAA:
	out << " &#1585;&#1575;";
	break;

      case TOK_SUFFIX_HAA:
	out << "&#1607;&#1575;";
	break;

      case TOK_SUFFIX_II:
	out << "&#1610;";
	break;


      case TOK_SPACE:
	out << ' ';
	break;

      case TOK_PERIOD:
	out << '.';
	break;

      case TOK_COMMA:
	out << ',';
	break;

      case TOK_SEMICOLON:
	out << ';';
	break;

      case TOK_COLON:
	out << ':';
	break;

      case TOK_EXCLAM:
	out << '!';
	break;

      case TOK_QUERY:
	out << '?';
	break;

      case TOK_PARAGRAPH:
	out << std::endl << std::endl;
	break;


      case TOK_SPACER:
	out << '-';
	break;


      case TOK_LEFT_QUOTE:
      case TOK_RIGHT_QUOTE:
	out << "&quot;";
	break;


#ifdef MODE_STACK
      case TOK_PUSH_MODE:
      case TOK_POP_MODE:
	break;
#endif // MODE_STACK


      case TOK_UNKNOWN:
	out << (char)letter->flags;
	break;

      default:
	std::cerr << "output_unicode: unhandled token "
		  << letter->token << std::endl;
	break;
      }
    }

    letter++;
  }
}

static inline void output_string(std::list<arabic_letter>::iterator letter,
				 std::ostream& out, const std::string& str,
				 bool maybe_add_shadda = false)
{
  const char *ptr = str.c_str();

  if (letter->flags & TF_CAPITALIZE)
    out << char(toupper(*ptr)) << ptr + 1;
  else
    out << ptr;

  if (maybe_add_shadda && letter->flags & TF_SHADDA)
    output_string(letter, out, str, false);
}

static inline void output_initial_string(std::list<arabic_letter>::iterator letter,
					 std::ostream& out, const std::string& str)
{
  if (letter->token == TOK_ALIF ||
      letter->token == TOK_AYN  ||
      letter->token == TOK_HAMZA)
    {
      output_string(letter, out, str);
    } else {
    out << str;
  }
}

void output_latex_house(std::list<arabic_letter>& in, std::ostream& out,
			arabic_mode_t mode)
{
  std::list<arabic_letter>::iterator letter = in.begin();

  while (letter != in.end()) {
    switch (letter->token) {
    case TOK_ALIF:
      if (letter->flags & TF_VOWEL) {
	out << "\\'{";
	output_string(letter, out, "a}");
      }
      break;

    case TOK_BIH:
      output_string(letter, out, "b", true);
      break;

    case TOK_TIH:
      output_string(letter, out, "t", true);
      break;

    case TOK_THIH:
      out << "\\underline{";
      output_string(letter, out, "th}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{th}";
      break;

    case TOK_JIIM:
      output_string(letter, out, "j", true);
      break;

    case TOK_HIH_HUTII:
      out << "\\d{";
      output_string(letter, out, "h}");

      if (letter->flags & TF_SHADDA)
	out << "\\d{h}";
      break;

    case TOK_KHIH:
      out << "\\underline{";
      output_string(letter, out, "kh}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{kh}";
      break;

    case TOK_SIIN:
      output_string(letter, out, "s", true);
      break;

    case TOK_SHIIN:
      out << "\\underline{";
      output_string(letter, out, "sh}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{sh}";
      break;

    case TOK_DAAL:
      output_string(letter, out, "d", true);
      break;

    case TOK_DHAAL:
      out << "\\underline{";
      output_string(letter, out, "dh}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{dh}";
      break;

    case TOK_RIH:
      output_string(letter, out, "r", true);
      break;

    case TOK_ZIH:
      output_string(letter, out, "z", true);
      break;

    case TOK_SAAD:
      out << "\\d{";
      output_string(letter, out, "s}");

      if (letter->flags & TF_SHADDA)
	out << "\\d{s}";
      break;

    case TOK_THAAD:
      out << "\\d{";
      output_string(letter, out, "d}");

      if (letter->flags & TF_SHADDA)
	out << "\\d{d}";
      break;

    case TOK_TAYN:
      out << "\\d{";
      output_string(letter, out, "t}");

      if (letter->flags & TF_SHADDA)
	out << "\\d{t}";
      break;

    case TOK_DTHAYN:
      out << "\\d{";
      output_string(letter, out, "z}");

      if (letter->flags & TF_SHADDA)
	out << "\\d{z}";
      break;

    case TOK_AYN:
      out << '`';
      break;

    case TOK_GHAYN:
      out << "\\underline{";
      output_string(letter, out, "gh}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{gh}";
      break;

    case TOK_FIH:
      output_string(letter, out, "f", true);
      break;

    case TOK_QAAF:
      output_string(letter, out, "q", true);
      break;

    case TOK_KAAF:
      output_string(letter, out, "k", true);
      break;

    case TOK_LAAM:
      output_string(letter, out, "l", true);
      break;

    case TOK_MIIM:
      output_string(letter, out, "m", true);
      break;

    case TOK_NUUN:
      output_string(letter, out, "n", true);
      break;

    case TOK_WAAW:
      if (letter->flags & TF_DIPHTHONG) {
	output_string(letter, out, "w", true);
      }
      else if (letter->flags & TF_CONSONANT) {
	output_string(letter, out, "v", true);
      }
      else if (letter->flags & TF_VOWEL) {
	if (letter->flags & TF_SILENT) {
	  output_string(letter, out, "aw"); // jww (2002-10-30): ???
	} else {
	  out << "\\'{";
	  output_string(letter, out, "u}");
	}
      }
      break;

    case TOK_YIH:
      if (letter->flags & (TF_CONSONANT | TF_DIPHTHONG)) {
	output_string(letter, out, "y", true);
      }
      else if (letter->flags & TF_VOWEL) {
	if (letter->flags & TF_CAPITALIZE)
	  out << "\\'{I}";
	else
	  out << "\\'{\\i}";
      }
      break;

    case TOK_ALIF_MAQSURA:
      out << "\\'{";
      output_string(letter, out, "a}");
      break;

    case TOK_HIH:
      output_string(letter, out, "h", true);
      break;

    case TOK_TIH_MARBUTA:
      output_string(letter, out, "t", true);
      break;

    case TOK_HAMZA:
      out << '\'';
      break;


      // Persian letters

    case TOK_PIH:
      output_string(letter, out, "p", true);
      break;

    case TOK_CHIH:
      out << "\\underline{";
      output_string(letter, out, "ch}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{ch}";
      break;

    case TOK_ZHIH:
      out << "\\underline{";
      output_string(letter, out, "zh}");

      if (letter->flags & TF_SHADDA)
	out << "\\underline{zh}";
      break;

    case TOK_GAAF:
      output_string(letter, out, "g", true);
      break;


    case TOK_PREFIX_AL:
      output_string(letter, out, "al-");
      break;

    case TOK_PREFIX_BI:
      output_string(letter, out, "bi-");
      break;

    case TOK_PREFIX_MII:
      output_string(letter, out, "m\\'{\\i}-");
      break;

    case TOK_SUFFIX_RAA:
      out << "-r\\'{a}";
      break;

    case TOK_SUFFIX_HAA:
      out << "-h\\'{a}";
      break;

    case TOK_SUFFIX_II:
      out << "\\'{\\i}";
      break;


    case TOK_SPACE:
      out << ' ';
      break;

    case TOK_PERIOD:
      out << '.';
      break;

    case TOK_COMMA:
      out << ',';
      break;

    case TOK_SEMICOLON:
      out << ';';
      break;

    case TOK_COLON:
      out << ':';
      break;

    case TOK_EXCLAM:
      out << '!';
      break;

    case TOK_QUERY:
      out << '?';
      break;

    case TOK_PARAGRAPH:
      out << std::endl
	  << std::endl;
      break;


    case TOK_SPACER:
      out << '-';
      break;


    case TOK_LEFT_QUOTE:
      out << "``";
      break;

    case TOK_RIGHT_QUOTE:
      out << "''";
      break;


#ifdef MODE_STACK
    case TOK_PUSH_MODE:
    case TOK_POP_MODE:
      break;		// we ignore it
#endif // MODE_STACK


    case TOK_UNKNOWN:
      out << (char)letter->flags;
      break;

    default:
      std::cerr << "output_latex_house: unhandled token "
		<< letter->token << std::endl;
      break;
    }

    std::list<arabic_letter>::iterator next = letter;
    next++;

    if (letter->flags & TF_CONSONANT &&
	(next == in.end() || ! (next->flags & TF_VOWEL))) {
      if (letter->flags & TF_FATHA) {
	output_initial_string(letter, out, "a");
      }
      else if (letter->flags & TF_KASRA) {
	if (next != in.end() && next->flags & TF_DIPHTHONG)
	  output_initial_string(letter, out, "a");
	else
	  output_initial_string(letter, out, "i");
      }
      else if (letter->flags & TF_DHAMMA) {
	if (next != in.end() && next->flags & TF_DIPHTHONG)
	  output_initial_string(letter, out, "a");
	else
	  output_initial_string(letter, out, "u");
      }
      else if (letter->flags & TF_DEFECTIVE_ALIF) {
	out << "\\'{";
	output_string(letter, out, "a}");
      }
    }
    else if (letter->flags & TF_IZAAFIH) {
      if (letter->flags & TF_VOWEL)
	out << "y-i-";
      else
	out << "-i-";

      if (next != in.end() && next->token == TOK_SPACE)
	letter++;
    }

    letter++;
  }
}

// Simplified conversion functions

typedef bool (*parse_func_t)(std::istream&, std::list<arabic_letter>&,
			     arabic_mode_t, bool only_one);
typedef void (*output_func_t)(std::list<arabic_letter>& in,
			      std::ostream& out, arabic_mode_t mode);

inline void arabic_convert(std::istream& in, std::ostream& out,
			   arabic_mode_t mode,
			   parse_func_t parse, output_func_t output)
{
  std::list<arabic_letter> tokens;
  (*parse)(std::cin, tokens, mode, false);
  (*output)(tokens, std::cout, mode);
}

inline std::string arabic_convert_str(const std::string& in,
				      arabic_mode_t mode,
				      parse_func_t parse,
				      output_func_t output)
{
  std::list<arabic_letter> tokens;

  std::istrstream sin(in.c_str());
  (*parse)(sin, tokens, mode, false);

  int len = in.length();
  len *= 10;
  char *buf = new char[len];

  std::ostrstream sout(buf, len - 1);
  (*output)(tokens, sout, mode);
  sout << '\0';

  std::string result = buf;
  delete[] buf;

  return result;
}

char *arabic_convert_cp(const char *in, arabic_mode_t mode,
			parse_func_t parse, output_func_t output)
{
  std::list<arabic_letter> tokens;

  std::istrstream sin(in);
  (*parse)(sin, tokens, mode, false);

  int len = strlen(in);
  len *= 10;
  char *buf = (char *) malloc(len);

  std::ostrstream sout(buf, len - 1);
  (*output)(tokens, sout, mode);
  sout << '\0';

  return buf;
}

#ifdef PYTHON_MODULE

#include "Python.h"

#define SCHEME_AASAAN 	   0
#define SCHEME_ARABTEX 	   1
#define SCHEME_LATEX_HOUSE 2

static parse_func_t parsers[] = {
  parse_aasaan,
  //parse_arabtex,
  //parse_latex_house
  parse_talattof
};

static output_func_t printers[] = {
  output_aasaan,
  output_arabtex,
  output_latex_house
};


static PyObject *arabic_python_convert(PyObject *self, PyObject *args)
{
  unsigned int parser_index, printer_index;
  arabic_mode_t mode = MODE_ARABIC;
  char *text, *result;

  if (! PyArg_ParseTuple(args, "sii|i", &text,
			 &parser_index, &printer_index, (int *) &mode)) {
    return NULL;
  }

  if (strlen(text) == 0 ||
      parser_index  >= (sizeof(parsers)  / sizeof(parse_func_t)) ||
      printer_index >= (sizeof(printers) / sizeof(output_func_t))) {
    return NULL;
  }

  result = arabic_convert_cp(text, (arabic_mode_t) mode,
			     parsers[parser_index],
			     printers[printer_index]);
  PyObject *value = Py_BuildValue("s", result);
  free(result);
  return value;
}

/************************************************************************/

// Define a name->function correlation table.

static PyMethodDef ArabicMethods[] = {
  {"convert", arabic_python_convert, 1,
   "This function will convert between different representations\n\
of the Arabic alphabet."}, /* the 1 is simply required */
  {NULL,      NULL}		/* Sentinel */
};


// When Python attempts to load this module via "import arabic", it
// will call `initarabic', which needs to register its methods with
// the interpreter.

extern "C" void initarabic()
{
  PyObject *module = Py_InitModule("arabic", ArabicMethods);

  PyModule_AddIntConstant(module, "ARABIC", 0);
  PyModule_AddIntConstant(module, "PERSIAN", 1);

  PyModule_AddIntConstant(module, "AASAAN", 0);
  PyModule_AddIntConstant(module, "ARABTEX", 1);
  PyModule_AddIntConstant(module, "LATEX_HOUSE", 2);
  PyModule_AddIntConstant(module, "TALATTOF", 3);
}

#endif // PYTHON_MODULE

#ifdef STANDALONE

int main(int argc, char *argv[])
{
  int argi = 1;
  if (argc == argi) {
    std::cerr << "usage: arabic [--arabic|--persian] --html|--latex|--house "
	      << std::endl;
    return 1;
  }

  arabic_mode_t mode = MODE_ARABIC;
  std::string option = argv[argi];
  if (option == "--arabic") {
    mode = MODE_ARABIC;
    argi++;
  }
  else if (option == "--persian") {
    mode = MODE_PERSIAN;
    argi++;
  }

  output_func_t renderer;
  option = argv[argi];
  if (option == "--unicode") {
    renderer = output_unicode;
    argi++;
  }
  else if (option == "--latex") {
    renderer = output_arabtex;
    argi++;
  }
  else if (option == "--latex-house") {
    renderer = output_latex_house;
    argi++;
  }
    
  std::list<arabic_letter> tokens;
  parse_aasaan(std::cin, tokens, mode);
  (*renderer)(tokens, std::cout, mode);

  return 0;
}

#endif // STANDALONE
