## Cache otimizado — mapa inativo

A source marca `cl1` como mapa ativo em `mapas/.troca.json`; o conjunto `_rio` é alternativo/inativo nesta build. O instalador continua baixando o mesmo ZIP completo e validando o SHA-256 original, mas pula a extração de `mapas/_rio`. Isso reduz o espaço instalado em aproximadamente 104 MB (arquivos extraídos). Não reduz o tamanho do download inicial do ZIP (aprox. 1,50 GB). Os arquivos restantes do cache continuam os mesmos; valide no aparelho antes de distribuir.
