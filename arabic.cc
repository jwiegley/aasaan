#include <list>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include "arabic.h"

namespace arabic {

// Input functions

inline
void aasaan_either_or(std::istream& in, std::list<element_t>& out,
                      token_t first, token_t second)
{
  char c;
  in.get(c);
  if (in.eof()) {
    out.push_back(element_t(first, TF_CONSONANT));
  } else {
    switch (c) {
    case 'h':
      out.push_back(element_t(second, TF_CONSONANT));
      break;

    default:
      in.unget();
    case '_':
      out.push_back(element_t(first, TF_CONSONANT));
      break;
    }
  }
}

/* Turn a stream of Aasaan transliterated text into unambiguous
   tokens, which can be used to render any form of output
   encoding. */

#ifdef MODE_STACK
static std::list<mode_t> mode_stack;
#endif

#define push(out, tok, flags) (out).push_back(element_t((tok), (flags)))

bool parse_aasaan(std::istream& in, std::list<element_t>& out,
                  mode_t mode, bool only_one = false)
{
  char c;
  unsigned int start_size = out.size();
  bool capitalize_next = false;

  in.get(c);
  while (! in.eof()) {
    element_t& last = out.back();

    switch (c) {
    case 'a': {
      bool parsed = false;

      in.get(c);
      if (! in.eof()) {
        switch (c) {
        case 'a':
          if (! is_letter(last))
	    push(out, ALIF, TF_VOWEL | TF_BAA_KULAA);
	  else
	    push(out, ALIF, TF_VOWEL);
          parsed = true;
          break;

#if 0
        case 'y':
	  last.flags |= TF_FATHA;
	  push(out, YIH, TF_CONSONANT | TF_DIPHTHONG);
          parsed = true;
          break;

        case 'w':
	  last.flags |= TF_FATHA;
	  push(out, WAAW, TF_CONSONANT | TF_DIPHTHONG);
          parsed = true;
          break;
#endif

        case 'l':
          if (! is_letter(last)) {
            in.get(c);
            if (! in.eof()) {
              if (c == '-') {
                parsed = true;
                push(out, PREFIX_AL, TF_NO_FLAGS);
                break;
              }
              in.unget();
            }
          }
          // else fall through...

        default: {
          in.unget();        // only parse the first 'a'
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

          if (is_letter(last) || last.token == SPACER ||
              ! parse_aasaan(in, out, mode, true))
            break;
                    
          element_t& prev = out.back();
          element_t  copy = prev;

          // If the parse below succeeds in finding another
          // character, it means we did not see a doubled
          // letter (TF_SHADDA).

          in.get(c);
          if (in.eof() || c != '-') {
            out.pop_back();
            in.seekg(before);
            break;
          }

          if (parse_aasaan(in, out, mode, true)) {
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
            push(out, PREFIX_AL, TF_SUN_LETTER);
            copy.flags |= TF_SHADDA;
            out.push_back(copy);
          }
          break;
        }
        }
      }

      if (! parsed) {
        if (is_letter(last))
          last.flags |= TF_FATHA;
        else
          push(out, ALIF, TF_CONSONANT | TF_CARRIER | TF_FATHA);
      }
      break;
    }

    case 'i': {
      bool parsed = false;

      in.get(c);
      if (! in.eof()) {
        switch (c) {
        case 'i':
          if (is_letter(last))
            last.flags |= TF_KASRA;
          else
            push(out, ALIF, TF_CONSONANT | TF_CARRIER | TF_KASRA);

          parsed = true;
          push(out, YIH, TF_VOWEL);
          break;

        default:
          in.unget();        // only parse the first 'i'
          break;
        }
      }

      if (! parsed) {
        if (is_letter(last))
          last.flags |= TF_KASRA;
        else
          push(out, ALIF,
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

          if (is_letter(last))
            last.flags |= TF_DHAMMA;
          else
            push(out, ALIF, TF_CONSONANT | TF_CARRIER | TF_DHAMMA);

          push(out, WAAW,
               c == 'u' ? TF_VOWEL : TF_CONSONANT | TF_DIPHTHONG);
        } else {
          in.unget();        // only parse the first 'u'
        }
      }

      if (! parsed) {
        if (is_letter(last))
          last.flags |= TF_DHAMMA;
        else
          push(out, ALIF,
               TF_CONSONANT | TF_CARRIER | TF_DHAMMA);
      }
      break;
    }

    case 'o':
      if (is_letter(last))
        last.flags |= TF_DHAMMA;
      push(out, WAAW, TF_VOWEL | TF_SILENT);
      break;

    case 'e':
      in.get(c);
      if (! in.eof()) {
        if (c == 'y') {
          if (is_letter(last))
            last.flags |= TF_KASRA;
          else
            push(out, ALIF, TF_CONSONANT | TF_CARRIER | TF_KASRA);
          push(out, YIH, TF_CONSONANT | TF_DIPHTHONG);
        } else {
          in.unget();
        }
      }
      break;

    case 'b':
      in.get(c);
      if (! in.eof()) {
        if (c == 'i') {
          in.get(c);
          if (! in.eof()) {
            if (c == '-') {
              push(out, PREFIX_BI, TF_NO_FLAGS);
              break;
            }
            in.unget();
          }
        }
        in.unget();
      }
      push(out, BIH, TF_CONSONANT);
      break;

    case 'p':
      push(out, PIH, TF_CONSONANT);
      break;

    case 't':
      aasaan_either_or(in, out, TIH, THIH);
      break;

    case 'j':
      push(out, JIIM, TF_CONSONANT);
      break;

    case 'c':
      in.get(c);
      if (! in.eof()) {
        if (c == 'h') {
          push(out, CHIH, TF_CONSONANT);
          break;
        }
        in.unget();
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

      if (mode == PERSIAN &&
          last.flags & TF_CONSONANT &&
          ! (last.flags & TF_DIPHTHONG) &&
          (in.eof() || c == '-' || isspace(c))) {
        flags |= TF_SILENT;
      }
      in.unget();

      push(out, HIH, flags);
      break;
    }

    case 'H':
      push(out, HIH, TF_CONSONANT);
      break;

    case 'k':
      aasaan_either_or(in, out, KAAF, KHIH);
      break;

    case 'd':
      aasaan_either_or(in, out, DAAL, DHAAL);
      break;

    case 's':
      aasaan_either_or(in, out, SIIN, SHIIN);
      break;

    case 'r':
      push(out, RIH, TF_CONSONANT);
      break;

    case 'z':
      aasaan_either_or(in, out, ZIH, ZHIH);
      break;

    case '`':
      in.get(c);
      if (! in.eof()) {
        if (c == '`') {
          push(out, LEFT_QUOTE, TF_NO_FLAGS);
          break;
        }
        in.unget();
      }
      push(out, AYN, TF_CONSONANT);
      break;

    case 'g':
      aasaan_either_or(in, out, GAAF, GHAYN);
      break;

    case 'f':
      push(out, FIH, TF_CONSONANT);
      break;

    case 'q':
      push(out, QAAF, TF_CONSONANT);
      break;

    case 'l':
      in.get(c);
      if (! in.eof()) {
        if (c == 'i') {
          in.get(c);
          if (! in.eof()) {
            if (c == '-') {
              push(out, PREFIX_LI, TF_NO_FLAGS);
              break;
            }
            in.unget();
          }
        }
        in.unget();
      }
      push(out, LAAM, TF_CONSONANT);
      break;

    case 'm':
      in.get(c);
      if (! in.eof()) {
        if (c == 'i') {
          in.get(c);
          if (! in.eof()) {
            if (c == 'i') {
              in.get(c);
              if (! in.eof()) {
                if (c == '-') {
                  push(out, PREFIX_MII, TF_NO_FLAGS);
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
      push(out, MIIM, TF_CONSONANT);
      break;

    case 'n':
      push(out, NUUN, TF_CONSONANT);
      break;

    case 'w':
      in.get(c);
      if (! in.eof()) {
        if (c == 'a') {
          in.get(c);
          if (! in.eof()) {
            if (c == '-') {
              push(out, PREFIX_WA, TF_NO_FLAGS);
              break;
            }
            in.unget();
          }
        }
        in.unget();
      }
      // fall through...
    case 'v':
      push(out, WAAW, TF_CONSONANT);
      break;

    case 'y':
      push(out, YIH, TF_CONSONANT);
      break;

    case '.':
      in.get(c);
      if (! in.eof()) {
        switch (c) {
        case 'h':
          push(out, HIH_HUTII, TF_CONSONANT);
          break;

        case 's':
          push(out, SAAD, TF_CONSONANT);
          break;

        case 'd':
          push(out, THAAD, TF_CONSONANT);
          break;

        case 't':
          push(out, TAYN, TF_CONSONANT);
          break;

        case 'z':
          push(out, DTHAYN, TF_CONSONANT);
          break;

        default:
          in.unget();
          push(out, PERIOD, TF_NO_FLAGS);
          break;
        }
      }
      break;

    case 'N':
      if (is_letter(last))
        last.flags |= TF_TANWEEN;
      break;

    case 'T':
      push(out, TIH_MARBUTA, TF_CONSONANT);
      break;

    case 'Y':
      push(out, ALIF_MAQSURA, TF_VOWEL);
      break;

    case '\'':
      in.get(c);
      if (! in.eof()) {
        if (c == '\'') {
          push(out, RIGHT_QUOTE, TF_NO_FLAGS);
          break;
        }
        in.unget();
      }
      push(out, HAMZA, TF_CONSONANT);
      break;

#ifdef MODE_STACK
    case 'A':
      in.get(c);
      if (! in.eof()) {
        if (c == '/') {
          mode_stack.push_back(mode = ARABIC);
          push(out, PUSH_MODE, mode);
          break;
        }
        in.unget();
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

          assert(is_letter(last));
          last.flags |= TF_DHAMMA;
          push(out, WAAW, TF_VOWEL | TF_SILENT_ALIF);
          break;
        }
        in.unget();
      }
      break;

#ifdef MODE_STACK
    case 'P':
      in.get(c);
      if (! in.eof()) {
        if (c == '/') {
          mode_stack.push_back(mode = PERSIAN);
          push(out, PUSH_MODE, mode);
          break;
        }
        in.unget();
      }
      break;

    case '/':
      in.get(c);
      if (! in.eof()) {
        if (c == 'A' || c == 'P') {
          if (! mode_stack.empty()) {
            mode = mode_stack.back();
            mode_stack.pop_back();
          }
          push(out, POP_MODE, mode);
        } else {
          in.unget();
	  push(out, UNKNOWN, (unsigned long) '/');
        }
      }
      break;
#endif // MODE_STACK

    case '^':
      capitalize_next = true;
      goto NEXT;

    case ',':
      push(out, COMMA, TF_NO_FLAGS);
      break;

    case ';':
      push(out, SEMICOLON, TF_NO_FLAGS);
      break;

    case ':':
      push(out, COLON, TF_NO_FLAGS);
      break;

    case '!':
      push(out, EXCLAM, TF_NO_FLAGS);
      break;

    case '?':
      push(out, QUERY, TF_NO_FLAGS);
      break;

    case '~':
      parse_aasaan(in, out, mode, true);
      out.back().flags |= TF_SHADDA;
      break;

    case '_':
      in.get(c);
      if (! in.eof()) {
        if (c == 'a') {
          if (is_letter(last))
            last.flags |= TF_DEFECTIVE_ALIF;
        } else {
          in.unget();
        }
      }
      break;

    case '-':
      in.get(c);
      if (! in.eof()) {
        if (! is_letter(last)) {
          in.unget();
        }
        else if (c == 'i') {
          in.get(c);
          if (in.eof()) {
            last.flags |= TF_IZAAFIH;
            break;
          } else {
            if (isspace(c)) {
              last.flags |= TF_IZAAFIH;
              in.unget();        // let it be separate words
              break;
            }
            else if (c == 'i') {
              in.get(c);
              if (in.eof()) {
                push(out, SUFFIX_II, TF_NO_FLAGS);
                break;
              } else {
                if (isspace(c)) {
                  push(out, SUFFIX_II, TF_NO_FLAGS);
                  in.unget();
                  break;
                }
                in.unget();
              }
            }
            in.unget();
          }
	  in.unget();
        }
        else if (c == 'r' || c == 'h') {
          int f = c;
          in.get(c);
          if (! in.eof()) {
            if (c == 'a') {
              in.get(c);
              if (! in.eof()) {
                if (c == 'a') {
                  in.get(c);
                  if (in.eof()) {
                    push(out, f == 'r' ? SUFFIX_RAA : SUFFIX_HAA,
                         TF_NO_FLAGS);
                    break;
                  } else {
                    if (isspace(c)) {
                      push(out, f == 'r' ? SUFFIX_RAA : SUFFIX_HAA,
                           TF_NO_FLAGS);
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
	  in.unget();
        } else {
          in.unget();
        }
      }
      push(out, SPACER, TF_NO_FLAGS);
      break;

    default:
      push(out, UNKNOWN, (unsigned long) c);
      break;

    case '\t':
    case ' ':
    case '\n': {
      // Gobble up multiple whitespace characters; they
      // collapse into SPACE, unless there are two returns,
      // in which case it is a paragraph separator

      int ret = 0;
      while (in.get(c) && ! in.eof() && isspace(c)) {
        if (c == '\n') ret++;
      }
      if (! in.eof())
        in.unget();

      if (ret > 1)
        push(out, PARAGRAPH, TF_NO_FLAGS);
      else
        push(out, SPACE, TF_NO_FLAGS);
      break;
    }
    }

    if (! (last.flags & TF_SHADDA) && is_sukun(last) &&
        &last != &out.back() && last == out.back())
      {
        last.flags |= TF_SHADDA;
        out.pop_back();

        // Check if this is a sun letter
        std::list<element_t>::iterator i = out.end();
        i--; assert(*i == out.back());
        i--;

        if (i->token == PREFIX_AL)
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

bool parse_talattof(std::istream& in, std::list<element_t>& out,
                    mode_t mode, bool only_one)
{
  char c;
  unsigned int start_size = out.size();

  in.get(c);
  while (! in.eof()) {
    element_t& last = out.back();

    switch (c) {
    case '\015': push(out, PARAGRAPH, TF_NO_FLAGS); break;

    case '\040': push(out, SPACE, TF_NO_FLAGS); break;
    case '\240': push(out, SPACE, TF_NO_FLAGS); break;
    case '\256': push(out, PERIOD, TF_NO_FLAGS); break;

#if 0
    case '\260': push(out, ZERO, TF_NO_FLAGS); break;
    case '\261': push(out, ONE, TF_NO_FLAGS); break;
    case '\262': push(out, TWO, TF_NO_FLAGS); break;
    case '\263': push(out, THREE, TF_NO_FLAGS); break;
    case '\264': push(out, FOUR, TF_NO_FLAGS); break;
    case '\265': push(out, FIVE, TF_NO_FLAGS); break;
    case '\266': push(out, SIX, TF_NO_FLAGS); break;
    case '\267': push(out, SEVEN, TF_NO_FLAGS); break;
    case '\270': push(out, EIGHT, TF_NO_FLAGS); break;
    case '\271': push(out, NINE, TF_NO_FLAGS); break;
#endif

    case '\302':
      push(out, ALIF, TF_VOWEL | TF_FATHA | TF_BAA_KULAA);
      break;
            
    case '\306':
      push(out, HAMZA, TF_CONSONANT | TF_KASRA);

    case '\307':
      if (is_letter(last))
        push(out, ALIF, TF_VOWEL | TF_FATHA);
      else
        push(out, ALIF, TF_CONSONANT | TF_FATHA);
      break;

    case '\310': push(out, BIH, TF_CONSONANT); break;
    case '\312': push(out, TIH, TF_CONSONANT); break;
    case '\313': push(out, THIH, TF_CONSONANT); break;
    case '\314': push(out, JIIM, TF_CONSONANT); break;
    case '\315': push(out, HIH_HUTII, TF_CONSONANT); break;
    case '\316': push(out, KHIH, TF_CONSONANT); break;
    case '\317': push(out, DAAL, TF_CONSONANT); break;
    case '\320': push(out, DHAAL, TF_CONSONANT); break;
    case '\321': push(out, RIH, TF_CONSONANT); break;
    case '\322': push(out, ZIH, TF_CONSONANT); break;
    case '\323': push(out, SIIN, TF_CONSONANT); break;
    case '\324': push(out, SHIIN, TF_CONSONANT); break;
    case '\325': push(out, SAAD, TF_CONSONANT); break;
    case '\326': push(out, THAAD, TF_CONSONANT); break;
    case '\327': push(out, TAYN, TF_CONSONANT); break;
    case '\330': push(out, DTHAYN, TF_CONSONANT); break;
    case '\331': push(out, AYN, TF_CONSONANT); break;
    case '\332': push(out, GHAYN, TF_CONSONANT); break;
    case '\341': push(out, FIH, TF_CONSONANT); break;
    case '\342': push(out, QAAF, TF_CONSONANT); break;
    case '\343': push(out, KAAF, TF_CONSONANT); break;
    case '\344': push(out, LAAM, TF_CONSONANT); break;
    case '\345': push(out, MIIM, TF_CONSONANT); break;
    case '\346': push(out, NUUN, TF_CONSONANT); break;
    case '\347': push(out, HIH, TF_CONSONANT); break;
    case '\350': push(out, WAAW, TF_CONSONANT); break;
    case '\351': push(out, YIH, TF_CONSONANT); break; // final
    case '\352': push(out, YIH, TF_CONSONANT); break;

    case '\354': push(out, HAMZA, TF_CONSONANT); break;

    case '\353':
      if (is_letter(last)) {
        if (last.token == ALIF) {
          out.pop_back();
          out.back().flags |= TF_FATHA | TF_TANWEEN;
        } else {
          last.flags |= TF_TANWEEN;
        }
      }
      break;

    case '\360':
      if (is_letter(last))
        last.flags |= TF_IZAAFIH | TF_EXPLICIT;
      break;

    case '\363': push(out, PIH, TF_CONSONANT); break;
    case '\370': push(out, GAAF, TF_CONSONANT); break;
    case '\376': push(out, ZHIH, TF_CONSONANT); break;

    default: push(out, EXCLAM, TF_NO_FLAGS); break;
    }

    if (! in.eof())
      in.get(c);
  }

  return start_size != out.size();
}

// Output functions

bool output_aasaan_letter(std::list<element_t>::iterator letter,
                          std::list<element_t>::iterator end,
                          std::ostream& out, mode_t mode,
                          bool quiet_shadda)
{
  switch (letter->token) {
  case ALIF:
    if (letter->flags & TF_VOWEL)
      out << 'a';
    break;

  case BIH:
    out << 'b';
    break;

  case TIH:
    out << 't';
    break;

  case THIH:
    out << "th";
    break;

  case JIIM:
    out << 'j';
    break;

  case HIH_HUTII:
    out << ".h";
    break;

  case KHIH:
    out << "kh";
    break;

  case SIIN:
    out << 's';
    break;

  case SHIIN:
    out << "sh";
    break;

  case DAAL:
    out << 'd';
    break;

  case DHAAL:
    out << "dh";
    break;

  case RIH:
    out << 'r';
    break;

  case ZIH:
    out << 'z';
    break;

  case SAAD:
    out << ".s";
    break;

  case THAAD:
    out << ".d";
    break;

  case TAYN:
    out << ".t";
    break;

  case DTHAYN:
    out << ".z";
    break;

  case AYN:
    out << '`';
    break;

  case GHAYN:
    out << "gh";
    break;

  case FIH:
    out << 'f';
    break;

  case QAAF:
    out << 'q';
    break;

  case KAAF:
    out << 'k';
    break;

  case LAAM:
    out << 'l';
    break;

  case MIIM:
    out << 'm';
    break;

  case NUUN:
    out << 'n';
    break;

  case WAAW:
    if (letter->flags & TF_DIPHTHONG) {
      out << 'w';
    }
    else if (letter->flags & TF_CONSONANT) {
      if (mode == ARABIC)
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

  case YIH:
    if (letter->flags & (TF_CONSONANT | TF_DIPHTHONG))
      out << 'y';
    else if (letter->flags & TF_VOWEL) {
      out << 'i';
    }
    break;

  case ALIF_MAQSURA:
    out << 'Y';
    break;

  case HIH:
    if (letter->flags & TF_SILENT || mode == ARABIC) {
      out << 'h';
    } else {
      std::list<element_t>::iterator prev = letter; prev--;
      std::list<element_t>::iterator next = letter; next++;

      if (is_sukun(*letter) &&
          (next == end || ! is_letter(*next)) &&
          ! (prev->flags & (TF_VOWEL | TF_DIPHTHONG |
                            TF_DEFECTIVE_ALIF)))
        {
          out << 'H';
        } else {
        out << 'h';
      }
    }
    break;

  case TIH_MARBUTA:
    out << 'T';
    break;

  case HAMZA:
    out << '\'';
    break;


    // Persian letters

  case PIH:
    out << 'p';
    break;

  case CHIH:
    out << "ch";
    break;

  case ZHIH:
    out << "zh";
    break;

  case GAAF:
    out << 'g';
    break;

  default:
    return false;
  }

  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  std::list<element_t>::iterator next = letter;
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
        (letter->token == SIIN ||
         letter->token == TIH ||
         letter->token == DAAL ||
         letter->token == KAAF ||
         letter->token == GAAF ||
         letter->token == ZIH) &&
        next->flags & TF_CONSONANT &&
        next->token == HIH)
      {
        out << '_';
      }
  }

  if (letter->flags & TF_TANWEEN)
    out << 'N';
    
  return true;
}

void output_aasaan(std::list<element_t>& in, std::ostream& out,
                   mode_t mode)
{
  std::list<element_t>::iterator letter = in.begin();

  while (letter != in.end()) {
    if (letter->flags & TF_CAPITALIZE)
      out << '^';
    if (letter->token == AYN && letter->flags & TF_SHADDA) {
      out << '~';
      output_aasaan_letter(letter, in.end(), out, mode, false);
    }
    else if (output_aasaan_letter(letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
        output_aasaan_letter(letter, in.end(), out, mode, false);
    }
    else {
      switch (letter->token) {
      case PREFIX_AL:
        if (! (letter->flags & TF_SUN_LETTER)) {
          out << "al-";
          break;
        }
        letter++;

        out << 'a';
        output_aasaan_letter(letter, in.end(), out, mode, true);
        out << '-';

        // when laam is sun, it must be output: al-ll
        if (letter->token == LAAM)
          output_aasaan_letter(letter, in.end(), out, mode, true);
        output_aasaan_letter(letter, in.end(), out, mode, false);

        break;

      case PREFIX_BI:
        out << "bi-";
        break;

      case PREFIX_LI:
        out << "li-";
        break;

      case PREFIX_WA:
        out << "wa-";
        break;

      case PREFIX_MII:
        out << "mii-";
        break;

      case SUFFIX_RAA:
        out << "-raa";
        break;

      case SUFFIX_HAA:
        out << "-haa";
        break;

      case SUFFIX_II:
        out << "-ii";
        break;


      case SPACE:
        out << ' ';
        break;

      case PERIOD:
        out << '.';
        break;

      case COMMA:
        out << ',';
        break;

      case SEMICOLON:
        out << ';';
        break;

      case COLON:
        out << ':';
        break;

      case EXCLAM:
        out << '!';
        break;

      case QUERY:
        out << '?';
        break;

      case PARAGRAPH:
        out << std::endl
            << std::endl;
        break;


      case SPACER:
        out << '-';
        break;


      case LEFT_QUOTE:
        out << "``";
        break;

      case RIGHT_QUOTE:
        out << "''";
        break;


#ifdef MODE_STACK
      case PUSH_MODE:
        if (letter->flags == (unsigned long) ARABIC) {
          mode = ARABIC;
          out << "A/";
        }
        else if (letter->flags == (unsigned long) PERSIAN) {
          mode = PERSIAN;
          out << "P/";
        }
        break;

      case POP_MODE:
        if (mode == ARABIC)
          out << "/A";
        else if (mode == PERSIAN)
          out << "/P";

        mode = (mode_t) letter->flags;
        break;
#endif // MODE_STACK


      case UNKNOWN:
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

bool output_arabtex_letter(std::list<element_t>::iterator letter,
                           std::list<element_t>::iterator end,
                           std::ostream& out, mode_t mode,
                           bool quiet_shadda)
{
  switch (letter->token) {
  case ALIF:
    if (letter->flags & TF_VOWEL)
      out << 'a';
    break;

  case BIH:
    out << 'b';
    break;

  case TIH:
    out << 't';
    break;

  case THIH:
    out << "_t";
    break;

  case JIIM:
    out << "^g";
    break;

  case HIH_HUTII:
    out << ".h";
    break;

  case KHIH:
    out << "_h";
    break;

  case SIIN:
    out << 's';
    break;

  case SHIIN:
    out << "^s";
    break;

  case DAAL:
    out << 'd';
    break;

  case DHAAL:
    out << "_d";
    break;

  case RIH:
    out << 'r';
    break;

  case ZIH:
    out << 'z';
    break;

  case SAAD:
    out << ".s";
    break;

  case THAAD:
    out << ".d";
    break;

  case TAYN:
    out << ".t";
    break;

  case DTHAYN:
    out << ".z";
    break;

  case AYN:
    out << '`';
    break;

  case GHAYN:
    out << ".g";
    break;

  case FIH:
    out << 'f';
    break;

  case QAAF:
    out << 'q';
    break;

  case KAAF:
    out << 'k';
    break;

  case LAAM:
    out << 'l';
    break;

  case MIIM:
    out << 'm';
    break;

  case NUUN:
    out << 'n';
    break;

  case WAAW:
    // If this waaw is followed by a silent alif, then the
    // encoding has already been done during the handling of the
    // preceding letter's DHAMMA, below
        
    if (! (letter->flags & TF_SILENT_ALIF))
      out << 'w';
    break;

  case YIH:
    out << 'y';
    break;

  case ALIF_MAQSURA:
    out << "Y_a";
    break;

  case HIH:
    if (letter->flags & TF_SILENT)
      out << 'H';
    else
      out << 'h';
    break;

  case TIH_MARBUTA:
    out << 'T';
    break;

  case HAMZA:
    out << '\'';
    break;


  // Persian letters

  case PIH:
    out << 'p';
    break;

  case CHIH:
    out << "^c";
    break;

  case ZHIH:
    out << "^z";
    break;

  case GAAF:
    out << 'g';
    break;

  default:
    return false;
  }

  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  std::list<element_t>::iterator next = letter;
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
    out << 'N';

  return true;
}

void output_arabtex(std::list<element_t>& in, std::ostream& out, mode_t mode)
{
  std::list<element_t>::iterator letter = in.begin();

  while (letter != in.end()) {
    if (output_arabtex_letter(letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
        output_arabtex_letter(letter, in.end(), out, mode, false);
    }
    else {
      switch (letter->token) {
      case PREFIX_AL:
        out << "al-";
        break;

      case PREFIX_BI:
        out << "bi-";
        break;

      case PREFIX_LI:
        out << "li-";
        break;

      case PREFIX_WA:
        out << "wa-";
        break;

      case PREFIX_MII:
        out << "mI\\hspace{0.4ex}";
        break;

      case SUFFIX_RAA:
        out << "\\hspace{0.4ex}raa";
        break;

      case SUFFIX_HAA:
        out << "-haa";
        break;

      case SUFFIX_II:
        out << "-I";
        break;


      case SPACE:
        out << ' ';
        break;

      case PERIOD:
        out << '.';
        break;

      case COMMA:
        out << ',';
        break;

      case SEMICOLON:
        out << ';';
        break;

      case COLON:
        out << ':';
        break;

      case EXCLAM:
        out << '!';
        break;

      case QUERY:
        out << '?';
        break;

      case PARAGRAPH:
        out << std::endl << std::endl;
        break;


      case SPACER:
        out << '-';
        break;


      case LEFT_QUOTE:
        out << "\\lq ";
        break;

      case RIGHT_QUOTE:
        out << "\\rq ";
        break;


#ifdef MODE_STACK
      case PUSH_MODE:
      case POP_MODE:
        if (letter->flags == (unsigned long) ARABIC)
          out << "\\setarab \\newtanwin";
        else if (letter->flags == (unsigned long) PERSIAN)
          out << "\\setfarsi \\newtanwin";
        break;
#endif // MODE_STACK


      case UNKNOWN:
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

bool output_unicode_letter(std::list<element_t>::iterator last,
			   std::list<element_t>::iterator letter,
                           std::list<element_t>::iterator end,
                           std::ostream& out, mode_t mode,
                           bool quiet_shadda)
{
  std::list<element_t>::iterator next = letter;
  next++;

  // 1. Hamza rules need to be worked out.

  bool wrote_alif = false;

  switch (letter->token) {
  case ALIF:
    if (letter->flags & TF_BAA_KULAA)
      out << "&#1570;";
    else
      out << "&#1575;";
    wrote_alif = true;
    break;

  case BIH:
    out << "&#1576;";
    break;

  case TIH:
    out << "&#1578;";
    break;

  case THIH:
    out << "&#1579;";
    break;

  case JIIM:
    out << "&#1580;";
    break;

  case HIH_HUTII:
    out << "&#1581;";
    break;

  case KHIH:
    out << "&#1582;";
    break;

  case SIIN:
    out << "&#1587;";
    break;

  case SHIIN:
    out << "&#1588;";
    break;

  case DAAL:
    out << "&#1583;";
    break;

  case DHAAL:
    out << "&#1584;";
    break;

  case RIH:
    out << "&#1585;";
    break;

  case ZIH:
    out << "&#1586;";
    break;

  case SAAD:
    out << "&#1589;";
    break;

  case THAAD:
    out << "&#1590;";
    break;

  case TAYN:
    out << "&#1591;";
    break;

  case DTHAYN:
    out << "&#1592;";
    break;

  case AYN:
    out << "&#1593;";
    break;

  case GHAYN:
    out << "&#1594;";
    break;

  case FIH:
    out << "&#1601;";
    break;

  case QAAF:
    out << "&#1602;";
    break;

  case KAAF:
    out << "&#1705;";
    break;

  case LAAM:
    out << "&#1604;";
    break;

  case MIIM:
    out << "&#1605;";
    break;

  case NUUN:
    out << "&#1606;";
    break;

  case WAAW:
    out << "&#1608;";
    break;

  case YIH:
    if (letter->flags & TF_IZAAFIH)
      out << "&#1574;";
    else if (mode == PERSIAN &&
	(next == end || next->token == SPACE ||
	 next->token == PERIOD || next->token == COMMA ||
	 next->token == RIGHT_QUOTE || next->token == SUFFIX_RAA ||
	 next->token == SUFFIX_HAA))
      out << "&#1740;";
    else if (mode == PERSIAN && next->token == SUFFIX_II)
      out << "&#1610;&#1740";
    else
      out << "&#1610;";
    break;

  case ALIF_MAQSURA:
    out << "&#1609;&#1648;";
    break;

  case HIH:
    if (letter->flags & TF_IZAAFIH && letter->flags & TF_SILENT) {
      out << "&#1728;";
    } else if (next->token == SUFFIX_II && letter->flags & TF_SILENT) {
      out << "&#1607;";
      out << "&#8239;";  // narrow no-break space
      out << "&#1575;";
      wrote_alif = true;
    } else {
      out << "&#1607;";
    }
    break;

  case TIH_MARBUTA:
    out << "&#1577;";
    break;

  case HAMZA:
    if (last == end || last->token == SPACE || last->token == LEFT_QUOTE) {
      if (letter->flags & TF_KASRA)
	out << "&#1573;";
      else
	out << "&#1571;";
    }
    else if (last != end && ! (last->flags & TF_DIPHTHONG) &&
	     (last->flags & TF_KASRA || letter->flags & TF_KASRA)) {
      out << "&#1574;";
    }
    else if (last != end && ! (last->flags & (TF_VOWEL | TF_DIPHTHONG)) &&
	     (last->flags & TF_DHAMMA || letter->flags & TF_DHAMMA)) {
      out << "&#1572;";
    }
    else if (last != end && ! (last->flags & TF_VOWEL) &&
	     (last->flags & TF_FATHA || letter->flags & TF_FATHA)) {
      out << "&#1571;";
      wrote_alif = true;
    }
    else {
      out << "&#1569;";
    }
    break;

  // Persian letters
  case PIH:
    out << "&#1662;";
    break;

  case CHIH:
    out << "&#1670;";
    break;

  case ZHIH:
    out << "&#1688;";
    break;

  case GAAF:
    out << "&#1711;";
    break;

  default:
    return false;
  }

  if (letter->flags & TF_TANWEEN) {
    if (letter->flags & TF_FATHA) {
      if (! wrote_alif)
	out << "&#1575;";
      out << "&#1611;";
    }
    else if (letter->flags & TF_KASRA) {
      out << "&#1613;";
    }
    else if (letter->flags & TF_DHAMMA) {
      out << "&#1612;";
    }
  } else if (mode == ARABIC || next == end || ! (next->flags & TF_VOWEL)) {
    if ((mode == ARABIC && next != end &&
	 next->flags & TF_VOWEL && next->token == ALIF) ||
	letter->flags & TF_FATHA)
      out << "&#1614;";
    else if ((mode == ARABIC && next != end &&
	      next->flags & TF_VOWEL && next->token == YIH) ||
	     letter->flags & TF_KASRA)
      out << "&#1616;";
    else if ((mode == ARABIC && next != end &&
	      next->flags & TF_VOWEL && next->token == WAAW) ||
	     letter->flags & TF_DHAMMA)
      out << "&#1615;";
    else if (mode == ARABIC &&
	     ! (letter->flags & TF_VOWEL) && ! (next->flags & TF_VOWEL) &&
	     ! (letter->flags & TF_DEFECTIVE_ALIF))
      out << "&#1618;";
  }

  if (letter->flags & TF_DEFECTIVE_ALIF)
      out << "&#1648;";

  if (letter->flags & TF_IZAAFIH) {
    if (letter->flags & TF_VOWEL && letter->token != YIH) {
      out << "&#1740;&#1616;";
    } else {
      out << "&#1616;";
    }
  }
  
  if (letter->flags & TF_SHADDA && quiet_shadda)
    return true;

  return true;
}

void output_unicode(std::list<element_t>& in, std::ostream& out,
                    mode_t mode)
{
  std::list<element_t>::iterator letter = in.begin();
  std::list<element_t>::iterator last = in.end();
  std::list<element_t>::iterator last_letter = in.end();

  while (letter != in.end()) {
    if (output_unicode_letter(last, letter, in.end(), out, mode, true)) {
      if (letter->flags & TF_SHADDA)
        out << "&#1617;";
    } else {
      switch (letter->token) {
      case PREFIX_AL:
	if (last_letter != in.end() &&
	    last_letter->flags & (TF_FATHA | TF_KASRA | TF_DHAMMA))
	  out << "&#1649;&#1604;&#1618;";
	else
	  out << "&#1571;&#1614;&#1604;&#1618;";
        break;

      case PREFIX_BI:
        out << "&#1576;&#1616;";
        break;

      case PREFIX_LI:
        out << "&#1604;&#1616;";
        break;

      case PREFIX_WA:
        out << "&#1608;&#1614;";
        break;

      case PREFIX_MII:
        out << "&#1605;&#1610;&#8239;";
        break;

      case SUFFIX_RAA:
        out << "&#8201;&#1585;&#1575;";
        break;

      case SUFFIX_HAA:
        out << "&#1607;&#1575;";
        break;

      case SUFFIX_II:
	if (mode == PERSIAN)
	  out << "&#1740;";
	else
	  out << "&#1610;";
        break;

      case SPACE: {
	std::list<element_t>::iterator next = letter;
	next++;
	if (! (next == in.end() || next->token == POP_MODE))
	  out << ' ';
        break;
      }

      case PERIOD:
        out << '.';
        break;

      case COMMA:
        out << ',';
        break;

      case SEMICOLON:
        out << '؛';
        break;

      case COLON:
        out << ':';
        break;

      case EXCLAM:
        out << '!';
        break;

      case QUERY:
        out << '؟';
        break;

      case PARAGRAPH:
        out << std::endl << std::endl;
        break;

      case SPACER:
        //out << "&#8201;";  // thin space
        out << "&#8239;";  // narrow no-break space
        break;

      case LEFT_QUOTE:
        out << "&#171;&#8201;";
        break;
      case RIGHT_QUOTE:
        out << "&#8201;&#187;";
        break;

#ifdef MODE_STACK
      case PUSH_MODE:
	mode_stack.push_front(mode);
	mode = (mode_t)letter->flags;
	break;
      case POP_MODE:
	mode = mode_stack.front();
	mode_stack.pop_front();
	break;
#endif // MODE_STACK

      case UNKNOWN:
        out << (char)letter->flags;
        break;

      default:
        std::cerr << "output_unicode: unhandled token "
                  << letter->token << std::endl;
        break;
      }
    }

    last = letter;
    if (is_letter(*letter))
      last_letter = letter;

    letter++;

    if (last->token == PUSH_MODE && letter != in.end() && letter->token == SPACE)
      letter++;
  }
}

static inline void output_string(std::list<element_t>::iterator letter,
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

static inline void output_initial_string(std::list<element_t>::iterator letter,
                                         std::ostream& out, const std::string& str)
{
  if (letter->token == ALIF ||
      letter->token == AYN  ||
      letter->token == HAMZA)
  {
    output_string(letter, out, str);
  } else {
    out << str;
  }
}

void output_latex_house(std::list<element_t>& in, std::ostream& out,
                        mode_t mode)
{
  std::list<element_t>::iterator letter = in.begin();

  while (letter != in.end()) {
    switch (letter->token) {
    case ALIF:
      if (letter->flags & TF_VOWEL) {
        out << "\\'{";
        output_string(letter, out, "a}");
      }
      break;

    case BIH:
      output_string(letter, out, "b", true);
      break;

    case TIH:
      output_string(letter, out, "t", true);
      break;

    case THIH:
      out << "\\underline{";
      output_string(letter, out, "th}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{th}";
      break;

    case JIIM:
      output_string(letter, out, "j", true);
      break;

    case HIH_HUTII:
      out << "\\d{";
      output_string(letter, out, "h}");

      if (letter->flags & TF_SHADDA)
        out << "\\d{h}";
      break;

    case KHIH:
      out << "\\underline{";
      output_string(letter, out, "kh}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{kh}";
      break;

    case SIIN:
      output_string(letter, out, "s", true);
      break;

    case SHIIN:
      out << "\\underline{";
      output_string(letter, out, "sh}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{sh}";
      break;

    case DAAL:
      output_string(letter, out, "d", true);
      break;

    case DHAAL:
      out << "\\underline{";
      output_string(letter, out, "dh}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{dh}";
      break;

    case RIH:
      output_string(letter, out, "r", true);
      break;

    case ZIH:
      output_string(letter, out, "z", true);
      break;

    case SAAD:
      out << "\\d{";
      output_string(letter, out, "s}");

      if (letter->flags & TF_SHADDA)
        out << "\\d{s}";
      break;

    case THAAD:
      out << "\\d{";
      output_string(letter, out, "d}");

      if (letter->flags & TF_SHADDA)
        out << "\\d{d}";
      break;

    case TAYN:
      out << "\\d{";
      output_string(letter, out, "t}");

      if (letter->flags & TF_SHADDA)
        out << "\\d{t}";
      break;

    case DTHAYN:
      out << "\\d{";
      output_string(letter, out, "z}");

      if (letter->flags & TF_SHADDA)
        out << "\\d{z}";
      break;

    case AYN:
      out << '`';
      break;

    case GHAYN:
      out << "\\underline{";
      output_string(letter, out, "gh}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{gh}";
      break;

    case FIH:
      output_string(letter, out, "f", true);
      break;

    case QAAF:
      output_string(letter, out, "q", true);
      break;

    case KAAF:
      output_string(letter, out, "k", true);
      break;

    case LAAM:
      output_string(letter, out, "l", true);
      break;

    case MIIM:
      output_string(letter, out, "m", true);
      break;

    case NUUN:
      output_string(letter, out, "n", true);
      break;

    case WAAW:
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

    case YIH:
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

    case ALIF_MAQSURA:
      out << "\\'{";
      output_string(letter, out, "a}");
      break;

    case HIH:
      output_string(letter, out, "h", true);
      break;

    case TIH_MARBUTA:
      output_string(letter, out, "t", true);
      break;

    case HAMZA:
      out << '\'';
      break;


      // Persian letters

    case PIH:
      output_string(letter, out, "p", true);
      break;

    case CHIH:
      out << "\\underline{";
      output_string(letter, out, "ch}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{ch}";
      break;

    case ZHIH:
      out << "\\underline{";
      output_string(letter, out, "zh}");

      if (letter->flags & TF_SHADDA)
        out << "\\underline{zh}";
      break;

    case GAAF:
      output_string(letter, out, "g", true);
      break;


    case PREFIX_AL:
      output_string(letter, out, "al-");
      break;

    case PREFIX_BI:
      output_string(letter, out, "bi-");
      break;

    case PREFIX_LI:
      output_string(letter, out, "li-");
      break;

    case PREFIX_WA:
      output_string(letter, out, "wa-");
      break;

    case PREFIX_MII:
      output_string(letter, out, "m\\'{\\i}-");
      break;

    case SUFFIX_RAA:
      out << "-r\\'{a}";
      break;

    case SUFFIX_HAA:
      out << "-h\\'{a}";
      break;

    case SUFFIX_II:
      out << "\\'{\\i}";
      break;


    case SPACE:
      out << ' ';
      break;

    case PERIOD:
      out << '.';
      break;

    case COMMA:
      out << ',';
      break;

    case SEMICOLON:
      out << ';';
      break;

    case COLON:
      out << ':';
      break;

    case EXCLAM:
      out << '!';
      break;

    case QUERY:
      out << '?';
      break;

    case PARAGRAPH:
      out << std::endl
          << std::endl;
      break;


    case SPACER:
      out << '-';
      break;


    case LEFT_QUOTE:
      out << "``";
      break;

    case RIGHT_QUOTE:
      out << "''";
      break;


#ifdef MODE_STACK
    case PUSH_MODE:
    case POP_MODE:
      break;                // we ignore it
#endif // MODE_STACK


    case UNKNOWN:
      out << (char)letter->flags;
      break;

    default:
      std::cerr << "output_latex_house: unhandled token "
                << letter->token << std::endl;
      break;
    }

    std::list<element_t>::iterator next = letter;
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

      if (next != in.end() && next->token == SPACE)
        letter++;
    }

    letter++;
  }
}

void output_html_house(std::list<element_t>& in, std::ostream& out,
                       mode_t mode)
{
  std::list<element_t>::iterator letter = in.begin();
  std::list<element_t>::iterator next = letter;
  next++;

  while (letter != in.end()) {
    switch (letter->token) {
    case ALIF:
      if (letter->flags & TF_VOWEL)
        output_string(letter, out, "á");
      break;

    case BIH:
      output_string(letter, out, "b", true);
      break;

    case TIH:
      output_string(letter, out, "t", true);
      break;

    case THIH:
      output_string(letter, out, "th");

      if (letter->flags & TF_SHADDA)
        out << "th";
      break;

    case JIIM:
      output_string(letter, out, "j", true);
      break;

    case HIH_HUTII:
      output_string(letter, out, "ḥ");

      if (letter->flags & TF_SHADDA)
        out << "ḥ";
      break;

    case KHIH:
      output_string(letter, out, "kh");

      if (letter->flags & TF_SHADDA)
        out << "kh";
      break;

    case SIIN:
      output_string(letter, out, "s", true);
      break;

    case SHIIN:
      output_string(letter, out, "sh");

      if (letter->flags & TF_SHADDA)
        out << "sh";
      break;

    case DAAL:
      output_string(letter, out, "d", true);
      break;

    case DHAAL:
      output_string(letter, out, "dh");

      if (letter->flags & TF_SHADDA)
        out << "dh";
      break;

    case RIH:
      output_string(letter, out, "r", true);
      break;

    case ZIH:
      output_string(letter, out, "z", true);
      break;

    case SAAD:
      output_string(letter, out, "ṣ");

      if (letter->flags & TF_SHADDA)
        out << "ṣ";
      break;

    case THAAD:
      output_string(letter, out, "ḍ");

      if (letter->flags & TF_SHADDA)
        out << "ḍ";
      break;

    case TAYN:
      output_string(letter, out, "ṭ");

      if (letter->flags & TF_SHADDA)
        out << "ṭ";
      break;

    case DTHAYN:
      output_string(letter, out, "ẓ");

      if (letter->flags & TF_SHADDA)
        out << "ẓ";
      break;

    case AYN:
      out << '`';
      break;

    case GHAYN:
      output_string(letter, out, "gh");

      if (letter->flags & TF_SHADDA)
        out << "gh";
      break;

    case FIH:
      output_string(letter, out, "f", true);
      break;

    case QAAF:
      output_string(letter, out, "q", true);
      break;

    case KAAF:
      output_string(letter, out, "k", true);
      break;

    case LAAM:
      output_string(letter, out, "l", true);
      break;

    case MIIM:
      output_string(letter, out, "m", true);
      break;

    case NUUN:
      output_string(letter, out, "n", true);
      break;

    case WAAW:
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
          output_string(letter, out, "ú");
        }
      }
      break;

    case YIH:
      if (letter->flags & (TF_CONSONANT | TF_DIPHTHONG)) {
        output_string(letter, out, "y", true);
      }
      else if (letter->flags & TF_VOWEL) {
        if (letter->flags & TF_CAPITALIZE)
          out << "Í";
        else
          out << "í";
      }
      break;

    case ALIF_MAQSURA:
      output_string(letter, out, "á");
      break;

    case HIH:
      output_string(letter, out, "h", true);
      break;

    case TIH_MARBUTA:
      output_string(letter, out, "t", true);
      break;

    case HAMZA:
      out << '\'';
      break;


      // Persian letters

    case PIH:
      output_string(letter, out, "p", true);
      break;

    case CHIH:
      output_string(letter, out, "ch");

      if (letter->flags & TF_SHADDA)
        out << "ch";
      break;

    case ZHIH:
      output_string(letter, out, "zh");

      if (letter->flags & TF_SHADDA)
        out << "zh";
      break;

    case GAAF:
      output_string(letter, out, "g", true);
      break;


    case PREFIX_AL:
      if (letter->flags & TF_SUN_LETTER)
	output_string(letter, out, "a-");
      else
	output_string(letter, out, "al-");
      break;

    case PREFIX_BI:
      output_string(letter, out, "bi-");
      break;

    case PREFIX_LI:
      output_string(letter, out, "li-");
      break;

    case PREFIX_WA:
      output_string(letter, out, "wa-");
      break;

    case PREFIX_MII:
      output_string(letter, out, "mí-");
      break;

    case SUFFIX_RAA:
      out << "-rá";
      break;

    case SUFFIX_HAA:
      out << "-há";
      break;

    case SUFFIX_II:
      out << "í";
      break;


    case SPACE:
      out << ' ';
      break;

    case PERIOD:
      out << '.';
      break;

    case COMMA:
      out << ',';
      break;

    case SEMICOLON:
      out << ';';
      break;

    case COLON:
      out << ':';
      break;

    case EXCLAM:
      out << '!';
      break;

    case QUERY:
      out << '?';
      break;

    case PARAGRAPH:
      out << std::endl
          << std::endl;
      break;


    case SPACER:
      out << '-';
      break;


    case LEFT_QUOTE:
      out << "“”";
      break;

    case RIGHT_QUOTE:
      out << "”";
      break;


#ifdef MODE_STACK
    case PUSH_MODE:
    case POP_MODE:
      break;                // we ignore it
#endif // MODE_STACK


    case UNKNOWN:
      out << (char)letter->flags;
      break;

    default:
      std::cerr << "output_latex_house: unhandled token "
                << letter->token << std::endl;
      break;
    }

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
        output_string(letter, out, "á");
      }
    }
    else if (letter->flags & TF_IZAAFIH) {
      if (letter->flags & TF_VOWEL)
        out << "y-i-";
      else
        out << "-i-";

      if (next != in.end() && next->token == SPACE)
        letter++;
    }

    letter++;
  }
}

// Simplified conversion functions

typedef bool (*parse_func_t)(std::istream&, std::list<element_t>&,
                             mode_t, bool only_one);
typedef void (*output_func_t)(std::list<element_t>& in,
                              std::ostream& out, mode_t mode);

void convert(std::istream& in, std::ostream& out, mode_t mode,
             parse_func_t parse, output_func_t output)
{
  std::list<element_t> tokens;
  (*parse)(std::cin, tokens, mode, false);
  (*output)(tokens, std::cout, mode);
}

std::string convert(const std::string& in, mode_t mode,
                    parse_func_t parse, output_func_t output)
{
  std::list<element_t> tokens;

  std::istringstream sin(in.c_str());
  (*parse)(sin, tokens, mode, false);

  std::ostringstream sout;
  (*output)(tokens, sout, mode);
  sout << '\0';

  return sout.str();
}

char * convert(const char *in, mode_t mode,
               parse_func_t parse, output_func_t output)
{
  static char buf[4096];

  std::list<element_t> tokens;

  std::istringstream sin(in);
  (*parse)(sin, tokens, mode, false);

  std::ostringstream sout;
  (*output)(tokens, sout, mode);
  sout << '\0';

  std::strcpy(buf, sout.str().c_str());

  return buf;
}

}

#ifdef USE_BOOST_PYTHON

#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <Python.h>

using namespace boost::python;
using namespace arabic;

namespace arabic {
  enum style_t {
    AASAAN,
    ARABTEX,
    UNICODE,
    LATEX_HOUSE,
    HTML_HOUSE,
    TALATTOF
  };
}

list py_parse(const std::string& in, style_t style, arabic::mode_t mode)
{
  std::list<element_t> elements;

  parse_func_t pf;
  switch (style) {
  case AASAAN:   pf = parse_aasaan;   break;
  case TALATTOF: pf = parse_talattof; break;
  default:
    return list();
  }

  std::istringstream sin(in.c_str());
  pf(sin, elements, mode, false);

  list py_elements;
  for (std::list<element_t>::iterator i = elements.begin();
       i != elements.end();
       i++)
    py_elements.append(*i);
  return py_elements;
}

std::string py_render(list args, style_t style, arabic::mode_t mode)
{
  std::list<element_t> elements;

  int l = len(args);
  for (int i = 0; i < l; i++)
    elements.push_back(extract<element_t>(args[i]));

  output_func_t of;
  switch (style) {
  case AASAAN:      of = output_aasaan;      break;
  case ARABTEX:     of = output_arabtex;     break;
  case UNICODE:     of = output_unicode;     break;
  case LATEX_HOUSE: of = output_latex_house; break;
  case HTML_HOUSE:  of = output_html_house;  break;
  default:
    return "";
  }

  std::ostringstream sout;
  of(elements, sout, mode);

  return sout.str();
}

BOOST_PYTHON_MODULE(arabic) {
  scope().attr("TF_NO_FLAGS")             = TF_NO_FLAGS;
  scope().attr("TF_CONSONANT")      = TF_CONSONANT;
  scope().attr("TF_CARRIER")             = TF_CARRIER;
  scope().attr("TF_VOWEL")             = TF_VOWEL;
  scope().attr("TF_DIPHTHONG")      = TF_DIPHTHONG;
  scope().attr("TF_SILENT")             = TF_SILENT;
  scope().attr("TF_FATHA")             = TF_FATHA;
  scope().attr("TF_KASRA")             = TF_KASRA;
  scope().attr("TF_DHAMMA")             = TF_DHAMMA;
  scope().attr("TF_DEFECTIVE_ALIF") = TF_DEFECTIVE_ALIF;
  scope().attr("TF_EXPLICIT")             = TF_EXPLICIT;
  scope().attr("TF_TANWEEN")             = TF_TANWEEN;
  scope().attr("TF_TANWEEN_ALIF")   = TF_TANWEEN_ALIF;
  scope().attr("TF_SILENT_ALIF")    = TF_SILENT_ALIF;
  scope().attr("TF_IZAAFIH")             = TF_IZAAFIH;
  scope().attr("TF_SUN_LETTER")     = TF_SUN_LETTER;
  scope().attr("TF_SHADDA")             = TF_SHADDA;
  scope().attr("TF_WITH_HAMZA")     = TF_WITH_HAMZA;
  scope().attr("TF_CAPITALIZE")     = TF_CAPITALIZE;
  scope().attr("TF_BAA_KULAA")      = TF_BAA_KULAA;

  enum_< token_t > ("token")
    .value("NONE",           NONE)
    .value("FIRST",           FIRST)
    .value("ALIF",           ALIF)
    .value("BIH",           BIH)
    .value("TIH",           TIH)
    .value("THIH",           THIH)
    .value("JIIM",           JIIM)
    .value("HIH_HUTII",    HIH_HUTII)
    .value("KHIH",           KHIH)
    .value("SIIN",           SIIN)
    .value("SHIIN",           SHIIN)
    .value("DAAL",           DAAL)
    .value("DHAAL",           DHAAL)
    .value("RIH",           RIH)
    .value("ZIH",           ZIH)
    .value("SAAD",           SAAD)
    .value("THAAD",           THAAD)
    .value("TAYN",           TAYN)
    .value("DTHAYN",       DTHAYN)
    .value("AYN",           AYN)
    .value("GHAYN",           GHAYN)
    .value("FIH",           FIH)
    .value("QAAF",           QAAF)
    .value("KAAF",           KAAF)
    .value("LAAM",           LAAM)
    .value("MIIM",           MIIM)
    .value("NUUN",           NUUN)
    .value("WAAW",           WAAW)
    .value("YIH",           YIH)
    .value("ALIF_MAQSURA", ALIF_MAQSURA)
    .value("HIH",           HIH)
    .value("TIH_MARBUTA",  TIH_MARBUTA)
    .value("HAMZA",           HAMZA)
    .value("PIH",           PIH)
    .value("CHIH",           CHIH)
    .value("ZHIH",           ZHIH)
    .value("GAAF",           GAAF)
    .value("LAST",           LAST)
    .value("PREFIX_AL",    PREFIX_AL)
    .value("PREFIX_BI",    PREFIX_BI)
    .value("PREFIX_LI",    PREFIX_LI)
    .value("PREFIX_WA",    PREFIX_WA)
    .value("PREFIX_MII",   PREFIX_MII)
    .value("SUFFIX_RAA",   SUFFIX_RAA)
    .value("SUFFIX_HAA",   SUFFIX_HAA)
    .value("SUFFIX_II",    SUFFIX_II)
    .value("PERIOD",       PERIOD)
    .value("COMMA",           COMMA)
    .value("SEMICOLON",    SEMICOLON)
    .value("COLON",           COLON)
    .value("EXCLAM",       EXCLAM)
    .value("QUERY",           QUERY)
    .value("SPACE",           SPACE)
    .value("PARAGRAPH",    PARAGRAPH)
    .value("LEFT_QUOTE",   LEFT_QUOTE)
    .value("RIGHT_QUOTE",  RIGHT_QUOTE)
    .value("SPACER",       SPACER)
#ifdef MODE_STACK
    .value("PUSH_MODE",    PUSH_MODE)
    .value("POP_MODE",     POP_MODE)
#endif
    .value("UNKNOWN",      UNKNOWN)
    .value("END",           END)
    ;

  class_ < element_t > ("Element")
    .def(init<token_t, optional<unsigned long> >())
    .def_readwrite("token", &element_t::token)
    .def_readwrite("flags", &element_t::flags)
    .def(self == self)
    .def(self != self)
    ;

  enum_< arabic::mode_t > ("mode")
    .value("ARABIC",  ARABIC)
    .value("PERSIAN", PERSIAN)
    ;

  enum_< style_t > ("style")
    .value("AASAAN",      AASAAN)
    .value("ARABTEX",     ARABTEX)
    .value("UNICODE",     UNICODE)
    .value("LATEX_HOUSE", LATEX_HOUSE)
    .value("HTML_HOUSE",  HTML_HOUSE)
    .value("TALATTOF",    TALATTOF)
    ;

  def("parse",  py_parse);
  def("render", py_render);
}

#endif // USE_BOOST_PYTHON

#ifdef STANDALONE

int main(int argc, char *argv[])
{
  int argi = 1;
  if (argc == argi) {
    std::cerr << "usage: arabic [--arabic|--persian] --html|--latex|--house "
              << std::endl;
    return 1;
  }

  arabic::mode_t mode = arabic::ARABIC;

  std::string option = argv[argi];
  if (option == "--arabic") {
    mode = arabic::ARABIC;
    argi++;
  }
  else if (option == "--persian") {
    mode = arabic::PERSIAN;
    argi++;
  }

  arabic::output_func_t renderer;
  option = argv[argi];
  if (option == "--unicode") {
    renderer = arabic::output_unicode;
    argi++;
  }
  else if (option == "--latex") {
    renderer = arabic::output_arabtex;
    argi++;
  }
  else if (option == "--latex-house") {
    renderer = arabic::output_latex_house;
    argi++;
  }
    
  std::list<arabic::element_t> tokens;
  arabic::parse_aasaan(std::cin, tokens, mode);
  (*renderer)(tokens, std::cout, mode);

  return 0;
}

#endif // STANDALONE
