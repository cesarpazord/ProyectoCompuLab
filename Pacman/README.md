# Pac-Man 3D — OpenGL

## Estructura de archivos

```
pacman3d/
├── main.cpp          ← Programa principal
├── Camera.h          ← Clase cámara FPS
├── Shader.h          ← Clase cargador de shaders GLSL
├── PacmanMesh.h      ← Geometría procedural de Pac-Man
└── Shader/
    ├── lighting.vs   ← Vertex shader (Phong)
    ├── lighting.frag ← Fragment shader (dir + point x4 + spot)
    ├── lamp.vs       ← Vertex shader cubo de luz
    └── lamp.frag     ← Fragment shader cubo de luz
```

---

## Dependencias

- OpenGL 3.3+
- GLEW
- GLFW 3
- GLM

### Windows (Visual Studio)
Instalar via vcpkg:
```
vcpkg install glew glfw3 glm
```

### Linux
```
sudo apt install libglew-dev libglfw3-dev libglm-dev
```

---

## Compilación

### Linux / Mac
```bash
g++ main.cpp -o pacman3d \
    -lGL -lGLEW -lglfw -lm \
    -std=c++17
```

### Windows (MinGW)
```
g++ main.cpp -o pacman3d.exe ^
    -lglew32 -lglfw3 -lopengl32 ^
    -std=c++17
```

---

## Controles

### Cámara (libre)
| Tecla | Acción |
|-------|--------|
| W / S | Adelante / Atrás |
| A / D | Izquierda / Derecha |
| Mouse | Rotar vista (yaw + pitch) |
| Scroll | Zoom (FOV) |

### Pac-Man
| Tecla | Acción |
|-------|--------|
| I / K | Avanzar / Retroceder (dirección que mira) |
| J / L | Moverse a los lados (strafe) |
| U / O | Subir / Bajar (eje Y) |
| ← → | Rotar en Yaw (izq/der) |
| ↑ ↓  | Rotar en Pitch (arriba/abajo) |
| R | Reset posición y rotación |
| ESPACIO | Toggle animación de boca |
| ESC | Salir |

---

## Descripción técnica

### PacmanMesh (geometría procedural)
- Esfera subdividida en `stacks x slices` (30x60 por defecto)
- La boca se genera **omitiendo los triángulos** que caen en el sector
  angular `±mouthAngle°` centrado en el eje +Z (frente del Pac-Man)
- Los **discos de la boca** (mandíbulas) se construyen como abanicos
  de triángulos en el plano de corte
- La malla se **regenera en GPU cada frame** con `GL_DYNAMIC_DRAW`
  solo cuando el ángulo cambia más de 0.3°

### Sistema de iluminación (lighting.frag)
- **Luz direccional**: simula el sol, difusa 0.7 amarillenta
- **PointLight[0]**: luz cálida (amarillo-naranja), desde arriba-derecha
- **PointLight[1]**: luz fría (azul), desde arriba-izquierda
- **SpotLight**: linterna azulada desde la cámara (cono 12.5° / 17.5°)

### Material Pac-Man
```
ambient  = (0.30, 0.28, 0.00)  ← amarillo oscuro
diffuse  = (1.00, 0.90, 0.00)  ← amarillo brillante
specular = (0.80, 0.75, 0.30)  ← reflejos dorados
shininess = 64
```

### Jerarquía de transformaciones
```
world
 └── pacPos (translate)
      └── pacYaw (rotate Y)
           └── pacPitch (rotate X)
                ├── PacmanMesh (scale 0.8)
                └── Ojo (translate local + scale 0.15)
```
