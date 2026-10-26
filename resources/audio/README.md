# Efeito de conquista do Glossata

`achievement.wav` foi produzido nesta subetapa por síntese aditiva determinística em `scripts/generate_achievement_sound.py`. Não utiliza gravações, samples, downloads, sons de plataformas de jogos ou modelos de áudio externos. Não se atribuem autoria humana, titularidade exclusiva ou revisão jurídica por inferência.

O gerador combina três vozes senoidais de 523,251 Hz, 659,255 Hz e 880 Hz, com entradas em 0, 95 e 190 ms, harmônicos fracos, ataque de 18 ms, decaimento exponencial e finalização de 160 ms. WAV PCM mono, 16 bits, 44.100 Hz e 0,85 segundo. Pico normalizado a aproximadamente −8,87 dBFS; RMS aproximado de −21,15 dBFS. Primeiro e último samples são zero. Esses valores não medem o volume físico das caixas de som.

Reprodução: `python scripts/generate_achievement_sound.py`; conferência sem alteração: `python scripts/generate_achievement_sound.py --check`. O JSON ao lado do WAV registra parâmetros e SHA-256, sem datas ou créditos fictícios.

**Licenciamento pendente:** o WAV gerado e seu procedimento estão documentados, mas não há nesta subetapa uma concessão expressa do mantenedor para esse novo recurso de áudio. A licença Apache-2.0 do escopo explícito de código não é aplicada automaticamente ao áudio. O gerador e os novos componentes têm sua pendência registrada separadamente em `docs/ESCOPO_LICENCIAMENTO.json`. A decisão sobre direitos disponíveis e condições de distribuição permanece humana. Nenhuma aprovação científica/jurídica ou permissão de distribuição é declarada.

O recurso é incorporado por `resources.qrc` e carregado por `QSoundEffect` em `qrc:/resources/audio/achievement.wav`; funciona offline. O gate de distribuição existente permanece obrigatório, incluindo as obrigações das dependências Qt Multimedia e do pacote efetivamente produzido.
