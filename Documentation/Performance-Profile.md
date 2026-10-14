# Kehityskoneen performance-profiili

`Config/DefaultEngine.ini` sisältää RTX 3060 Ti / 8 Gt VRAM -kehityskoneelle palautettavan low-memory-profiilin. Se ei muuta alkuperäisiä assetteja.

- Lumen, hardware ray tracing ja Virtual Shadow Maps ovat pois käytöstä.
- Renderöintiresoluutio on 75 % ja ruudunpäivitys rajataan 60 FPS:ään.
- Texture streaming -pooli on 1536 Mt ja tekstuurit käyttävät yhtä mip-tasoa kevyempää versiota.
- Skeletal mesh- ja static mesh -LOD vaihtuvat aikaisemmin.
- Varjot, heijastukset, foliage, efektit ja post process käyttävät low/medium-tasoa.
- Editorin raskaat asset-valmistelut suoritetaan yksi kerrallaan 6 Gt:n budjetilla.

Laatua voidaan myöhemmin nostaa poistamalla tai muuttamalla `Balanced low-memory graphics profile` -kommentin alla olevia CVar-rivejä. RendererSettings-osion Lumen-, ray tracing- ja Virtual Shadow Map -arvot voidaan samalla palauttaa arvoon 1.

Renderer-asetusten muutokset vaativat editorin uudelleenkäynnistyksen ja voivat käynnistää shaderien uudelleenvalmistelun.
