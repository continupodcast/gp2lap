# Ventana secundaria para batallas: investigación 0.15

Estado: **pendiente; no implementada en el ejecutable entregado**.
Objetivo solicitado: mostrar otra batalla cuando la diferencia entre dos
pilotos sea inferior a 0,1 segundos, manteniendo la cámara principal.

## Evidencia en el ejecutable aportado

Direcciones expresadas en el espacio de código usado por GP2Lap/IDA, no como
offsets de archivo ni direcciones del proceso RetroArch.

- `frankasm.inc` intercepta `0x717c9` en la copia de la cámara externa.
  En `0x717a2` comienza la rutina que toma un buffer, prepara la copia y llama
  a `0x715ec` y `0x71713`. Ese hook recibe una imagen ya construida.
- La ruta de cockpit usa la llamada de `0x7181e` a `0x71890`.
  El HUD usa antes/después de esa copia para restaurar su rectángulo.
- `0x713d6` selecciona las rutas de copia `0x717a2` y `0x71825` según el modo
  de vídeo. No ofrece un parámetro que seleccione una segunda cámara.
- La referencia de datos `0xa07c`, resuelta también desde la instrucción en
  `0x3b4c0`, participa en varias rutinas de dibujo. Cambiarla aisladamente no
  demuestra que se haya aislado el estado del renderizador.
- La rutina en `0x66080` guarda/restaura algunas referencias gráficas pero
  también modifica numerosas variables globales y procesa índices de objetos.
  No se ha identificado como una entrada segura de renderizado de otra cámara.

Duplicar o recortar el buffer actual mostraría la misma cámara; no cumple el
objetivo. Tampoco se ha validado que cambiar momentáneamente el auto seleccionado
y volver a ejecutar las rutinas sea independiente de la simulación y del HUD.

## Trabajo pendiente

1. Localizar la preparación de cámara y la entrada del renderizador de escena.
2. Identificar y preservar sus variables globales, buffers auxiliares, viewport,
   selección de auto, clipping y estado de los espejos.
3. Verificar una segunda imagen aislada dentro de DOSBox/RetroArch, sin avanzar
   la simulación ni alterar la cámara principal.
4. Medir el coste de esa pasada adicional y definir su frecuencia de actualización.
5. Integrar la selección de batalla con el umbral de 0,1 s y un retardo de salida
   para evitar aparición/desaparición continua cerca del umbral.

El entorno de esta entrega dispone de compilador y pruebas host, pero no de una
ejecución del juego en RetroArch/DOSBox para validar esa segunda pasada. No hay
evidencia suficiente para afirmar que sea imposible, ni para entregarla como
función operativa.
