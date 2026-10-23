# TTT Expansion DLC Rebuild V2.0.3 — Textos externos

## Como editar textos SEM recompilar

1. Instale a nova pasta `TTT_EXPANSION` do artefato GitHub Actions `TTT-Expansion-PRX-REBUILD-V2003` (incluindo subpasta `lang`).
2. Abra o PPSSPP > **Arquivo > Abrir pasta do Memory Stick**. Vá a `PSP/PLUGINS/TTT_EXPANSION/lang/`.
3. Edite `pt.txt` (português), `en.txt` (inglês) e/ou `es.txt` (espanhol) no Bloco de Notas/VS Code, em **UTF-8**.
4. Altere APENAS o texto após `=`. Exemplo: `TAB_CHARACTERS=PERSONAGENS` pode virar `TAB_CHARACTERS=ELENCO`.
5. Reinicie COMPLETAMENTE o PPSSPP e abra o menu. O PRX lê os arquivos quando inicializa o desenho (na primeira abertura).
6. Se um arquivo estiver ausente ou incorreto, a interface usa os textos padrão sem travar.

**IMPORTANTE:** a fonte de PSP não contém glifos acentuados. O loader converte automaticamente acentos latinos comuns para ASCII (ç→c, ã→a). Frases muito longas podem ser cortadas na tela 480x272. O formato é **TXT simples**, não JSON.

### Nova interface

- Aba **PERSONAGENS** (antes GOKUS).
- Abas **MISSOES**, **MAPAS** e **OPCOES** indicam **EM BREVE** e NÃO chamam ações inacabadas. O código das seis missões manuais foi preservado para depois, mas está desativado no menu.
- Rodapé separado em 3 linhas, com ações reais: **ESQ/DIR mover, CIMA/BAIXO alternar foco, L/R jogador; X confirmar, TRIANGULO aba, SELECT idioma; QUADRADO limpar, CIRCULO fechar**.
- Personagens, IDs e 12 CHRs preservados, mantendo a ISO expandida `0x3734`.

### Edição de textura (a ferramenta já existe)

- `tools/convert_menu_skin.py` converte `TTT_EXPANSION/ui/menu_bg_HD.png` e `font_atlas.png` em `.4444`.
- Requer **Python 3 + Pillow** (`py -3 -m pip install pillow`).
- Execute na raiz: `py -3 tools/convert_menu_skin.py`.
- Copie os arquivos `.4444` resultantes para o mesmo `ui/` do PSP (não é necessário recompilar PRX).
- A V2.0.3 continua usando internamente o atlas compartilhado de 128 KB da V2.0.2. A edição dos PNGs não é substituição automática em alta resolução; o conversor reduz para o formato PSP.

### Diagnóstico

O log `diagnostico.txt` agora registra:
`TEXT LANG=0x00000000 open=... bytes=... overrides=... invalid=...`
`LANG=0` é português, `1` inglês e `2` espanhol. `overrides>0` significa que valores foram lidos do `.txt`. Caminhos esperados: `ms0:/PSP/PLUGINS/TTT_EXPANSION/lang/pt.txt`, `en.txt`, `es.txt`.

### Limite de transformações (pesquisa pendente)

IDs externos 00, 01 e 02 já podem ser selecionados pelo nosso menu/injetor, mas **todos ainda apontam à forma interna 0x47** por segurança. Transformação dentro da batalha é um mecanismo distinto: tabela de transformação, gatilho, reaproveitamento/recarga de CHR, efeitos, animação e atualização sincronizada de ID/Color. Tentar apontar um comando de transformação ao ID02 não garante que carregará sem travar. Nenhum hook novo de transformações foi incluído nesta versão.

### Estado dos testes

Testes nativos de parser/idiomas, menu, CHRs, injeção, GPU simulada e PACKFILE executados em PC. A compilação PSP e a validação no PPSSPP precisam ser executadas pelo workflow e pelo usuário. **Mantenha backup do PRX V2.0.2 funcional.**
