##### **scopo**
scegli il livello di difficoltà del gioco, per superare il gioco deve trovare il coniglio e cliccare con tasto destro il coniglo

##### **difficoltà**

1. "man mano che sposta la telecamera, una parte del muro diventa trasparente fino a essere tutta trasparente" -> Non è realmente trasparente: **è a causa del clipping del muro**
2. "la collisione prenseta solo sul coniglio presenta, divente rosso e non sul il muro" -> **è a causa di wall\_model = translate \* scale \* cube.to\_unit\_extent; mancanza di cube.to\_unit\_extent;**
3. "per usare due shaders differenti, genera errere del tipo OpenGL" -> **perché**

   * **glGetUniformLocation() → non richiede che lo shader sia attivo.**
   * **glUniform...() → richiede che sia attivo** il programma shader a cui appartiene la location.
   * Ho dovuto separarli in funzioni diversi, creare ogni funzioni per ogni shader attivo
4. "quando cerco di mischiare la parte2D con la parte3D genera errore" -> SFML documenta esplicitamente che quando si mescolano rendering SFML e OpenGL **bisogna salvare/ripristinare gli stati OpenGL**; in SFML 3.0.2 sono disponibili:

   * **pushGLStates(),**
   * **popGLStates()**
   * **e resetGLStates()**



##### tappa01

###### disegnare il menu, i bottoni di livelli, e le parole (tutti allineati al centro)

##### 

##### tappa02

###### quando il mouse si sposta sul bottone del livello, il bottone del livello e il numero del livero cambia il colore



##### tappa03

###### disegnare il muro e il coniglio utilizzabili per tutti i livelli, implementare lo spostamento della camera in base a quale bottone cliccato.



##### tappa04

###### implementare il bounding box, impedire all’osservatore di spostarsi in quel punto e il programma genera una forma di feedback visivo.



##### tappa05

###### realizzare il picking del coniglio e spostare il coniglio dietro il muro. (livello 2)



##### tappa06

###### realizzare i 3 livelli di gioco e si cambia con i tasti 1, 2, 3.



##### tappa07

###### mischiare 2D con il 3D e il cambiamento del livello.



##### tappa08

###### il teapot si può spostare con i tasti w a s d.



##### tappa09

###### cambia il colore se il mouse si trova sul coniglio oppure sul teiera.



##### tappa10

il menu 2, quando utente clicca sul tasto h (help) per dare aiuto (indicazione). disegnare un messaggio di successo nel riquadro di menu.

---

##### Codice esterno

* ###### usare funzioni da laboratori vecchi, o prendere spunto dal

\#include "include/matrices.hh"

\#include "include/mesh.hh"

\#include "include/hotshaders.hh"

\#include "include/rawmouse.hh"

* ###### funzioni create da IA

  * Box world\_bounds (const glm::mat4\& model) const {}
  * bool mouse\_su\_bunny (sf::Vector2i position, sf::Vector2u window\_size) {}
  * bool mouse\_su\_sphere (sf::Vector2i position, sf::Vector2u window\_size) {}
  * bool ray\_triangle\_intersection(const glm::vec3\& ray\_origin, const glm::vec3\& ray\_direction, const glm::vec3\& v0, const glm::vec3\& v1,  const glm::vec3\& v2) {}
