##### **scopo**
scegli il livello di difficoltà del gioco, per superare il gioco deve trovare il coniglio e cliccare con tasto destro il coniglo

##### **difficoltà**

1. "man mano che sposta la telecamera, una parte del muro diventa trasparente fino a essere tutta trasparente" -> Non è realmente trasparente: **è a causa del clipping del muro**
2. "la collisione prenseta solo sul coniglio presenta, divente rosso e non sul il muro" -> **è a causa di wall\_model = translate \* scale \* cube.to\_unit\_extent; mancanza di cube.to\_unit\_extent;**
3. "per usare due shaders differenti, genera errere del tipo OpenGL" -> **perché**

   * **glGetUniformLocation() → non richiede che lo shader sia attivo.**
   * **glUniform...() → richiede che sia attivo** il programma shader a cui appartiene la location.
   * Ho dovuto separarli in funzioni diversi, creare ogni funzioni per ogni shader attivo




##### tappa01

###### disegnare il muro e il coniglio utilizzabili per tutti i livelli, implementare lo spostamento della camera in base a quale bottone cliccato.



##### tappa02

###### implementare il bounding box, impedire all’osservatore di spostarsi in quel punto e il programma genera una forma di feedback visivo.



##### tappa03

###### realizzare il picking del coniglio e spostare il coniglio dietro il muro. (livello 2)



##### tappa04

###### il teapot si può spostare con i tasti w a s d. Resize della camera. Cambiare livello 1,2,3 in base ai tasti 1,2,3



##### tappa05

###### cambia il colore se il mouse si trova sul coniglio oppure sul teiera.



##### tappa06

###### il menu 2, quando utente clicca sul tasto h (help) per dare aiuto. dare un messaggio di benvenuto e istruzione generale. Quando cambio i livello la posizione della camera ritorna come default.



##### tappa07

###### Miglioramento del picking, bool mouse_su (sf::Vector2i position, sf::Vector2u window_size, int n) diventa una funzione generale. Il mouse clicca su wall ha priorità sul mouse su bunny, per evitare al livello 2 se user "trova" il coniglio, nonostante è stato coperto da wall.



##### tappa08

###### Design del livello 4.



##### tappa09

###### Design del livello 5.

---

##### Codice esterno

* ###### usare funzioni da laboratori vecchi, o prendere spunto dal

\#include "include/matrices.hh"

\#include "include/mesh.hh"

\#include "include/hotshaders.hh"

\#include "include/rawmouse.hh"

* ###### funzioni create da IA

  * Box world\_bounds (const glm::mat4\& model) const {}
  * bool mouse\_su\ (sf::Vector2i position, sf::Vector2u window\_size, int n) {}
  * bool ray\_triangle\_intersection(const glm::vec3\& ray\_origin, const glm::vec3\& ray\_direction, const glm::vec3\& v0, const glm::vec3\& v1,  const glm::vec3\& v2) {}
