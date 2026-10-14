# GAS C++ -rungon käyttö UE 5.8:ssa

## Arkkitehtuuri

- AGameProjectSummer26PlayerState omistaa yhden replikoidun UGameProjectSummer26AbilitySystemComponent-komponentin ja yhden UGameProjectSummer26AttributeSet-olion.
- AGameProjectSummer26Character toimii ASC:n avatarina ja välittää IAbilitySystemInterface-kutsut PlayerStateen.
- Actor info alustetaan palvelimella PossessedBy-kutsussa ja omistavalla instanssilla OnRep_PlayerState-kutsussa.
- AGameProjectSummer26PlayerController::PostProcessInput käsittelee tagipohjaisen ability-inputin ilman Character Tickiä.
- Oletusattribuutit asetetaan kertaluonteisella UGameProjectSummer26DefaultAttributesEffect-effectillä.

Rungon attribuutit ovat:

- Health / MaxHealth
- Stamina / MaxStamina
- Mana / MaxMana
- Endurance / MaxEndurance

Kaikki kuusi ovat replikoituja, käyttävät RepNotifya ja pysyvät välillä 0-Max.

## Valmis Sprint-toiminto

- Vasemman Shiftin pitäminen aktivoi UGameProjectSummer26SprintAbility-abilityn.
- Kävelynopeus muuttuu arvosta 500 arvoon 900 sprintin ajaksi.
- Sprint kuluttaa 10 Staminaa sekunnissa GAS GameplayEffect -pulsseilla.
- Sprint loppuu, kun Shift vapautetaan tai Stamina saavuttaa nollan.
- Stamina palautuu sprintin ulkopuolella viisi pistettä sekunnissa.
- Sprint grantataan automaattisesti Input.Ability.Primary-tagilla.

Jos Character Blueprintiin asetetaan myöhemmin oma Primary Ability Action, se korvaa automaattisen Left Shift -fallbackin.

## Dodge ja Endurance

- C aktivoi `UGameProjectSummer26DodgeAbility`-abilityn, kun hahmo on maassa.
- Väistö suuntautuu viimeisimmän liikesyötteen suuntaan; ilman liikesyötettä hahmo väistää aina taaksepäin.
- Yksi dodge kuluttaa 50 Endurancea.
- Endurancen maksimi on 100, joten käytössä on kaksi 50 pisteen dodge-segmenttiä.
- Endurance palautuu 10 pistettä sekunnissa eli yksi kokonainen segmentti viidessä sekunnissa.
- Dodge ei aktivoidu, jos Endurancea on alle 50.
- Game Animation Samplen crouch-input on siirretty C:stä X:ään `IMC_Sandbox`-assetissa.
- `<` kääntää paikallisen hahmon ja kameran välittömästi 180 astetta.

## Crouch ja slide

- Vasen Ctrl pitää hahmon kyykyssä, kun sprintti ei ole aktiivinen. Sama näppäin on mapattu samplen `IA_Crouch`-actioniin motion-matching-animaatioita varten.
- Sprintissä Ctrl aloittaa maata pitkin tapahtuvan slidauksen, jos vauhtia on vähintään 500.
- Slide katkaisee sprintin ja sen Stamina-kulutuksen, lukitsee alkuperäisen liikesuunnan ja lisää alkuun 300 nopeutta (900 sprintistä 1200 slideen).
- Slide hidastuu 600 yksikköä sekunnissa ja päättyy nopeuden saavuttaessa 50, jolloin vaakanopeus asetetaan nollaan. Jos Ctrl on edelleen pohjassa, hahmo jää kyykkyyn; muuten hahmo nousee.
- Vanha X-crouch-mapping on poistettu `IMC_Sandbox`-assetista.

Valmista erillistä dodge-montagea ei ole kytketty. Ensimmäinen toimiva versio käyttää liikesuuntaista `LaunchCharacter`-väistöä ja samplen locomotion-animaatiota.

Dodge kulkee noin 280 Unreal-yksikköä ja päättyy aina vaakanopeuteen 0. Dodgen lopussa myös jonossa oleva liikesyöte tyhjennetään, joten jälkiliukua ei jää.

## Status-UI

UGameProjectSummer26StatusWidget luodaan automaattisesti jokaiselle paikalliselle pelaajalle. Se näyttää ruudun vasemmassa yläkulmassa:

- punaisen HP-palkin;
- vihreän Stamina-palkin;
- sinisen Mana-palkin;
- kaksi keltaista Dodge/Endurance-segmenttiä Mana-palkin alla;
- jokaisen resurssin nykyisen ja maksimiarvon.

Widget on toteutettu kokonaan C++:lla eikä vaadi Widget Blueprintia. PlayerControllerin Status Widget Class -kenttään voi myöhemmin asettaa oman aliluokan.

## Oman abilityn luominen

1. Luo C++- tai Blueprint-luokka, jonka parent on UGameProjectSummer26GameplayAbility.
2. Valitse Activation Policy:
   - On Input Triggered: yksi aktivointiyritys painalluksesta.
   - While Input Active: ability voidaan aktivoida uudelleen niin kauan kuin painike on pohjassa ja edellinen instanssi ei ole aktiivinen.
3. Lisää abilityn omassa toteutuksessa tarvittavat cost- ja cooldown-GameplayEffectit.
4. Kutsu abilityn aktivoinnissa CommitAbility.
5. Päätä jokainen onnistunut, epäonnistunut ja peruttu suoritus EndAbility-kutsulla.

## Startup-abilityjen rekisteröinti

Native PlayerState käyttää tarkoituksella tyhjää startup-listaa. Luo tarvittaessa Blueprint-luokka AGameProjectSummer26PlayerState-luokasta ja aseta se aktiivisen GameModen Player State Class -arvoksi.

Lisää Blueprint PlayerStaten Startup Abilities -listaan:

- ability-luokka;
- Input.Ability.Primary tai Input.Ability.Secondary.

Palvelin granttaa listan vain kerran. Sama ability-luokka tarkistetaan ennen GiveAbility-kutsua, joten possession-päivitys ei luo toista spec-riviä.

Vaihtoehtoisesti täytä StartupAbilities omassa C++ PlayerState -aliluokassa.

## Enhanced Input -kytkentä

Sprint toimii ilman uusia assetteja: jos Primary Ability Action on tyhjä, Character luo runtime-actionin ja mapittaa Left Shiftin prioriteetilla 10.

Oman Input Action -assetin voi kytkeä näin:

1. Luo haluamasi Input Action -asset tai käytä olemassa olevaa.
2. Lisää se aktiiviseen Input Mapping Contextiin ja valitse näppäin/peliohjaimen painike.
3. Aseta action aktiivisen Character Blueprintin kenttään:
   - Primary Ability Action
4. Käytä startup-abilityn tagina Input.Ability.Primary.

Lisätageille voi käyttää Characterin BlueprintCallable-funktioita:

- AbilityInputTagPressed
- AbilityInputTagReleased

ASC:n AbilityInputTagPressed ja AbilityInputTagReleased ovat käytettävissä myös suoraan C++:sta.

## Attribuuttien käyttö

AttributeSet löytyy PlayerStatesta esimerkiksi näin:

    const AGameProjectSummer26PlayerState* PS =
        Character->GetPlayerState<AGameProjectSummer26PlayerState>();
    const UGameProjectSummer26AttributeSet* Attributes =
        PS ? PS->GetCoreAttributeSet() : nullptr;

Muuta attribuutteja GameplayEffecteillä. Älä ylläpidä samoille arvoille rinnakkaisia Character-floatteja tai kirjoita resource-arvoja suoraan abilityn aktivointikoodissa.

Oletusarvot ovat 100. Jos projekti tarvitsee eri arvot, tee oma GameplayEffect ja aseta se PlayerState-aliluokan Default Attributes Effect -arvoksi.

## Build

Projektin editoritarget rakennetaan VS Coden olemassa olevalla GameProjectSummer26Editor Win64 Development -build-taskilla.

C++- ja UHT-build on varmennettu. Input Action-, Mapping Context- ja omien abilityjen runtime-toiminta pitää varmentaa niiden lisäämisen jälkeen projektin normaalissa PIE-ajossa.

## Game Animation Sample: GameMode_1

Projektissa ovat nyt valmiina:

- `/Game/Blueprints/BP_PlayerCharacter_1`, joka on kopio sample-projektin `SandboxCharacter_CMC`-hahmosta ja säilyttää sen liikkumis-, animaatio-, kamera- ja traversal-logiikan;
- `/Game/Blueprints/GameMode_1`, jonka Default Pawn Class on `BP_PlayerCharacter_1`, Player Controller Class on samplen `PC_Sandbox` ja Player State Class on C++-luokka `GameProjectSummer26PlayerState`.

Ota pelimuoto käyttöön halutulla kartalla:

1. Avaa kartta Unreal Editorissa.
2. Avaa **Window > World Settings**.
3. Aseta **GameMode Override** arvoksi `GameMode_1`.
4. Paina Play.

PIE-testissä varmista:

1. World Outlinerissa pelaajan pawn on `BP_PlayerCharacter_1`.
2. Samplen liike ja animaatiot toimivat WASD-, hiiri- ja hyppysyötteillä.
3. Ruudun vasemmassa yläkulmassa näkyvät HP-, Stamina- ja Mana-palkit arvolla 100/100.
4. Vasen Shift aktivoi GAS-sprintin; Stamina laskee 10 pistettä sekunnissa.
5. Shiftin vapauttamisen jälkeen Stamina palautuu 5 pistettä sekunnissa.

`Scripts/CreateGameAnimationGASAssets.py` rakentaa ja konfiguroi assetit uudelleen tarvittaessa. Se ajetaan Unreal Python -commandletilla tai Editorin Python-konsolista.

Headless-runtime-testissä on varmennettu, että `GameMode_1` lataa oikean pawnin, GAS:n avatar on `BP_PlayerCharacter_1`, sprint-ability grantataan ja attribuutit sekä status-HUD alustuvat. Varsinainen animaation visuaalinen laatu ja näppäinsyötteet varmennetaan PIE:ssä.
