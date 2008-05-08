#ifndef _ARABIC_H
#define _ARABIC_H

namespace arabic {

enum token_t {
  NONE,

  FIRST,

  // Arabic letters
  ALIF,
  BIH,
  TIH,
  THIH,
  JIIM,
  HIH_HUTII,
  KHIH,
  SIIN,
  SHIIN,
  DAAL,
  DHAAL,
  RIH,
  ZIH,
  SAAD,
  THAAD,
  TAYN,
  DTHAYN,
  AYN,
  GHAYN,
  FIH,
  QAAF,
  KAAF,
  LAAM,
  MIIM,
  NUUN,
  WAAW,
  YIH,
  ALIF_MAQSURA,
  HIH,
  TIH_MARBUTA,
  HAMZA,

  // Persian letters
  PIH,
  CHIH,
  ZHIH,
  GAAF,

  LAST,

  PREFIX_AL,
  PREFIX_BI,
  PREFIX_LI,
  PREFIX_WA,
  PREFIX_MII,
  SUFFIX_RAA,
  SUFFIX_HAA,
  SUFFIX_II,		// indefinite, or 2nd prs. copula

  PERIOD,
  COMMA,
  SEMICOLON,
  COLON,
  EXCLAM,
  QUERY,
  SPACE,
  PARAGRAPH,

  LEFT_QUOTE,
  RIGHT_QUOTE,

  SPACER,			// harmless "-"

#ifdef MODE_STACK
  PUSH_MODE,
  POP_MODE,
#endif

  UNKNOWN,
  END
};

enum mode_t { ARABIC, PERSIAN };

#define TF_NO_FLAGS 	  0x00000000

#define TF_CONSONANT 	  0x00000001
#define TF_CARRIER 	  0x00000002
#define TF_VOWEL     	  0x00000004
#define TF_DIPHTHONG  	  0x00000008
#define TF_SILENT    	  0x00000010

#define TF_FATHA 	  0x00000020
#define TF_KASRA 	  0x00000040
#define TF_DHAMMA 	  0x00000080
#define TF_DEFECTIVE_ALIF 0x00000100
#define TF_EXPLICIT       0x00000200

#define TF_TANWEEN    	  0x00000400
#define TF_TANWEEN_ALIF	  0x00000800
#define TF_SILENT_ALIF	  0x00001000
#define TF_IZAAFIH    	  0x00002000
#define TF_SUN_LETTER 	  0x00004000
#define TF_SHADDA     	  0x00008000
#define TF_WITH_HAMZA 	  0x00010000
#define TF_CAPITALIZE 	  0x00020000
#define TF_BAA_KULAA 	  0x00040000

struct element_t {
  token_t 	token;
  unsigned long flags;

  element_t() : token(NONE), flags(TF_NO_FLAGS) { }
  element_t(const element_t& other)
    : token(other.token), flags(other.flags) { }
  element_t(token_t tok, unsigned long fl = TF_NO_FLAGS)
    : token(tok), flags(fl) { }

  element_t& operator=(const element_t& other) {
    token = other.token;
    flags = other.flags;
    return *this;
  }

  bool operator==(const element_t& other) {
    return token == other.token && flags == other.flags;
  }
  bool operator!=(const element_t& other) {
    return ! (*this == other);
  }
};

inline bool is_letter(const element_t& elem) {
  return elem.token > FIRST && elem.token < LAST;
}

inline bool is_sukun(const element_t& elem) {
  return ! (elem.flags &
	    (TF_FATHA | TF_KASRA | TF_DHAMMA | TF_DEFECTIVE_ALIF));
}

}

#endif // _ARABIC_H
