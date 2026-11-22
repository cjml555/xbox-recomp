# Features: MCLA Recomp — Android

**Alcance final definido por el usuario** (20 nov 2026). Este documento captura el spec completo del app. El roadmap por fases está en `PLAN.md` (sección "Roadmap de Features").

## Menú de inicio

- **Selector de archivos del juego:** cargar los archivos del juego (ISO/datos extraídos por el usuario).
- **Menú de partida personalizada (custom save):** cargar saves personalizados.
- **Updater / verificador de actualizaciones:** comprobar e instalar actualizaciones del app.
- **Menú de gráficos:**
  - **Medidor FPS tipo MangoHud:** overlay de FPS/rendimiento en pantalla.
  - **Loader de drivers Vulkan públicos:** cargar drivers Vulkan públicos + catálogo de drivers públicos y custom.
  - **Selector de renderizador:** dos opciones — uno basado en Xenia y otro nativo/Vulkan.
  - **Selector de API gráfica.**
  - **Configuración de calidad con perfiles:** bajo, medio, alto, ultra y custom (tomando configs del juego).
- **Selector de idiomas.**
- **Menú de cheats.**
- **Menú de mods.**
- **Modo LAN (de ser posible):** multijugador LAN con gamertag personalizada.
- **Créditos.**
- **Botón para enviar logs de crash al dev.**

## Integrado en el app

- **Generador de shaders integrado:** multi-dispositivo (generación/adaptación de shaders por GPU/dispositivo).
- **Soporte de gamepads físicos:** Xbox, PlayStation y similares.
- **Gamepad virtual táctil:** visible y funcional cuando NO se detecta un gamepad físico.

## Modo de desarrollo

- **Modo de desarrollo** integrado en el app (herramientas de depuración/desarrollo).

## Prioridad de desarrollo (orden explícito del usuario)

1. **Renderizadores primero:** el juego debe renderizar y ser jugable (cargar y entrar al juego) ANTES de cualquier mejora. Implementar los renderizadores antes de todo.
2. **Optimización:** una vez que renderice, optimizar el código y la carga de archivos.
3. **Texturas:** al terminar la optimización, investigar cómo mejorar las texturas del juego.

## Restricciones legales

- **Los datos del juego NO se pueden subir ni publicar** (ni ISO, ni XEX, ni assets extraídos).
- **SÍ se pueden crear scripts** para parchar o descomprimir los archivos del juego (el usuario aporta sus propios archivos).
