enum arabic_token_t {
  TOK_NONE,

  TOK_FIRST,

  // Arabic letters
  TOK_ALIF,
  TOK_BIH,
  TOK_TIH,
  TOK_THIH,
  TOK_JIIM,
  TOK_HIH_HUTII,
  TOK_KHIH,
  TOK_SIIN,
  TOK_SHIIN,
  TOK_DAAL,
  TOK_DHAAL,
  TOK_RIH,
  TOK_ZIH,
  TOK_SAAD,
  TOK_THAAD,
  TOK_TAYN,
  TOK_DTHAYN,
  TOK_AYN,
  TOK_GHAYN,
  TOK_FIH,
  TOK_QAAF,
  TOK_KAAF,
  TOK_LAAM,
  TOK_MIIM,
  TOK_NUUN,
  TOK_WAAW,
  TOK_YIH,
  TOK_ALIF_MAQSURA,
  TOK_HIH,
  TOK_TIH_MARBUTA,
  TOK_HAMZA,

  // Persian letters
  TOK_PIH,
  TOK_CHIH,
  TOK_ZHIH,
  TOK_GAAF,

  TOK_LAST,

  TOK_PREFIX_AL,
  TOK_PREFIX_BI,
  TOK_PREFIX_MII,
  TOK_SUFFIX_RAA,
  TOK_SUFFIX_HAA,
  TOK_SUFFIX_II,		// indefinite, or 2nd prs. copula

  TOK_PERIOD,
  TOK_COMMA,
  TOK_SEMICOLON,
  TOK_COLON,
  TOK_EXCLAM,
  TOK_QUERY,
  TOK_SPACE,
  TOK_PARAGRAPH,

  TOK_LEFT_QUOTE,
  TOK_RIGHT_QUOTE,

  TOK_SPACER,			// harmless "-"

#ifdef MODE_STACK
  TOK_PUSH_MODE,
  TOK_POP_MODE,
#endif

  TOK_UNKNOWN,
  TOK_END
};

enum arabic_mode_t {
  MODE_ARABIC,
  MODE_PERSIAN
};

#define arabic_is_letter(let)				\
  ((let).token > TOK_FIRST && (let).token < TOK_LAST)

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

#define arabic_is_sukun(let)				\
  (! ((let).flags & (TF_FATHA | TF_KASRA |		\
		     TF_DHAMMA | TF_DEFECTIVE_ALIF)))

#define TF_TANWEEN    	  0x00000400
#define TF_TANWEEN_ALIF	  0x00000800
#define TF_SILENT_ALIF	  0x00001000
#define TF_IZAAFIH    	  0x00002000
#define TF_SUN_LETTER 	  0x00004000
#define TF_SHADDA     	  0x00008000
#define TF_WITH_HAMZA 	  0x00010000
#define TF_CAPITALIZE 	  0x00020000
#define TF_BAA_KULAA 	  0x00040000

struct arabic_letter {
  arabic_token_t token;
  unsigned long flags;

  arabic_letter(void) : token(TOK_NONE), flags(TF_NO_FLAGS) { }
  arabic_letter(const arabic_letter& other)
    : token(other.token), flags(other.flags) { }
  arabic_letter(arabic_token_t tok, unsigned long fl)
    : token(tok), flags(fl) { }

  arabic_letter& operator=(const arabic_letter& other) {
    token = other.token;
    flags = other.flags;
    return *this;
  }

  bool operator==(const arabic_letter& other) {
    return token == other.token && flags == other.flags;
  }
  bool operator!=(const arabic_letter& other) {
    return ! (*this == other);
  }
};
