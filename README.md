# TTT Expansion PRX V0.8 — Seletor para 1P–4P e retratos maiores

Baseado diretamente na **V0.7 que o usuário confirmou funcionando no PPSSPP**. Preserva a captura gráfica GE, o hook de entrada e a combinação L+R+Quadrado.

## IDs externos planejados — referência Tenkaichi 3

| Entrada | ID hexadecimal | Situação em Tenkaichi Tag Team (ULUS10537) |
|---|---|---|
| Goku Early (Saiyajin) | `00` | `FF` nas tabelas originais de recursos A/B |
| Goku Mid Base (Namek) | `01` | `FF` nas tabelas originais de recursos A/B |
| Goku Mid SSJ | `02` | `FF` nas tabelas originais de recursos A/B |
| Goku presente no TTT | `03` | Índice de recursos `00` nas tabelas A/B |

Os IDs 00 e 01 agora aparecem no menu da V0.8. O ID 02 está reservado para futura forma SSJ do Goku Mid. **O plugin NÃO cria personagens jogáveis nem escreve IDs na batalha.** Confirmar com X marca a escolha para o jogador no menu e no log, sem modificar o jogo. Antes de tornar os novos IDs jogáveis, precisaremos preencher os mapeamentos, arquivos CHR/SFX e tratar estruturas limitadas a 70 formas.

## Navegação intuitiva

- `L + R + Quadrado`: abre/fecha o seletor.
- **↑/↓: escolhe Goku Early ou Mid** (mesma direção da lista vertical).
- **←/→: escolhe Color 1–4** (cores em fila horizontal).
- **L/R isolados: muda 1P–4P**; a escolha de personagem e traje é guardada em cada slot.
- `X`: confirma em modo de prévia (não aplica ao jogo).
- `Triângulo`: muda Personagens/Mapas/Ajustes.
- `Select`: idioma PT/EN/ES.
- `Círculo`: fecha.

O bloqueio do controle no jogo base permanece ativo enquanto o menu do PRX estiver aberto. Como o código original só intercepta a chamada de controle identificada em 0881A158, essa proteção deve continuar sendo validada em todas as telas.

## Retratos e a pasta TEXTURES do PPSSPP

O plugin carrega arquivos externos em `PSP/PLUGINS/TTT_EXPANSION/portraits/`, **não** na pasta de substituição de texturas do PPSSPP.

Aceita `.rgba` crus (RGBA8888 sem cabeçalho):
- **128×128 = 65.536 bytes**, preferido para retratos.
- **64×64 = 16.384 bytes**, compatibilidade V0.7.

O plugin identifica o tamanho pelo comprimento exato do arquivo. Para converter PNG/JPG:

```cmd
py -m pip install pillow
py tools\converter_retratos.py goku_early.png TTT_EXPANSION\portraits\goku_early.rgba 128
py tools\converter_retratos.py goku_mid.png TTT_EXPANSION\portraits\goku_mid.rgba 128
```

Basta substituir os `.rgba` e reiniciar o PPSSPP, sem recompilar o PRX. O PNG por si só não é lido pelo plugin.

A **pasta `PSP/TEXTURES/ULUS10537`** é destinada ao recurso separado de substituição de texturas do PPSSPP para recursos do *jogo*. Ela não substitui automaticamente os `.rgba` do PRX. As duas técnicas podem coexistir: texturas do jogo com maior resolução na pasta TEXTURES; retratos personalizados do plugin na pasta portraits.

## Compilar e instalar

1. Substitua os arquivos do GitHub pelos do pacote V0.8, preservando os caminhos e a pasta `.github`.
2. Rode `Actions -> Compilar PRX do menu`.
3. Baixe o artifact **TTT-Expansion-PRX-V080**.
4. Copie a pasta `TTT_EXPANSION` gerada para `PSP/PLUGINS/` e reinicie o PPSSPP.
5. Confira `diagnostico.txt` após abrir o menu: registro `TTT V0.8` e `PORTRAITS bits=0x00000003`.

**Atenção:** ainda não executamos esta versão no PSPSDK/GitHub Actions nem no PPSSPP. Testes locais validam o estado do menu, mas desenho/GPU e hooks requerem validação em jogo. Faça backup da V0.7.
