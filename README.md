# ⛅: Raytracing diorama

Este proyecto es un diorama hecho con cubos, representando una escena de el videojuego Minecraft. Este proyecto implementa la tecnica de Raytracing y texturas sobre los cubos. Esta hehco en el lenguaje C++, con el uso de las librerias de SDL2, SLD_Image y GLM.

## 🔻 Requisitos del Sistema
- C++ 20 o superior
- SDL 2.0
- SDL Image
- GLM
  
## ⬇️ Clonación

- Clona el repositorio con el siguiente comando:
  ```bash
  git clone https://github.com/Diego2250/Raytracing.git
  ```
- Asegúrate de tener todas las dependencias instaladas.
- Compila el proyecto utilizando tu sistema de compilación preferido.

## 🎮 Controles

- **Flecha izquierda**: Girar a la izquierda
- **Flecha derecha**: Girar a la derecha
- **Flecha arriba**: Zoom in
- **Flecha abajo**: Zoom out


## 🎦 Video
https://github.com/Diego2250/Raytracing/assets/77738746/0b3c64aa-1ce9-440b-bde8-d2aa22090cae

## 💯 Rúbrica
- [✔️][30 puntos] Criterio subjetivo. Por qué tan compleja sea su escena
- [✔️][20 puntos] Criterio subjetivo. Por qué tan visualmente atractiva sea su escena
- [✔️][20 puntos] Por implementar rotación en su diorama y dejar que la camara se acerque y aleje
- [✔️ (Obsidiana, tierra, vortex de protal, hierro, diamante, lava, piedra)][5 puntos] por cada material diferente que implementen, para un máximo de 5 (piensen en los diferentes tipos de bloques en minecraft)
  - Para que el material cuente, debe tener su propia textura, y sus propios parametros para albedo, specular, transparencia y reflectividad
- [✔️][10 puntos] por implementar refracción en al menos uno de sus materiales (debe tener sentido contextual en su escena)
- [✔️][5 puntos] por implementar reflexión en al menos uno de sus materiales
- [✔️][20 puntos] por implementar un skybox para su material
