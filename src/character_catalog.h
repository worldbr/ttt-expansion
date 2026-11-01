#ifndef TTT_CHARACTER_CATALOG_H
#define TTT_CHARACTER_CATALOG_H
/* Carrega 01=Nome de characters.txt. Nomes podem ser editados sem recompilar PRX. */
void ttt_catalog_reload(void);
const char *ttt_catalog_name(unsigned slot);
#endif
