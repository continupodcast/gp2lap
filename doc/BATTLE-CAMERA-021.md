# Camara de batallas: etapa de factibilidad (no es una entrega DOS)

## Especificacion acordada

- Tecla 2: sustituir el log visual por ON/OFF; mensajes `Battling camera ON`
  y `Battling camera OFF`. OFF cierra la ventana y desarma la autoaparicion.
- Carrera, con vista principal TV u onboard. Camara secundaria TV.
- Misma zona que el mapa 6, ancho 300 px, altura proporcional.
- Ocultar el mapa solo mientras haya una segunda imagen valida y restaurarlo
  al cerrar. La composicion no debe modificar la seleccion persistente del mapa.
- Leyenda `NNN BATTLING FOR P`: iniciales del perseguidor y puesto disputado.
- Gap estrictamente menor a 200 ms durante mas de 5000 ms continuos.
- Cerrar al adelantar, superar 1000 ms o desactivar manualmente.
- Priorizar el puesto mas cercano al lider entre candidatos elegibles.
- Supuestos del prototipo: tiempos de simulacion; conservar la batalla elegida
  hasta su cierre; descartar boxes, abandonos, doblados y muestras no fiables.

## Trabajo realizado

`src/f1battle.c/h` implementa el controlador portable, sin llamadas al juego.
`tools/test_f1_battle.c` verifica los umbrales exactos, continuidad, prioridad,
adelantamiento, OFF manual, repeticion de tecla, discontinuidades y duplicados.
Prueba ejecutada con GCC C89 y advertencias tratadas como errores: aprobada.

El controlador NO esta incluido en makefile ni conectado a keyhand.c.
La tecla 2 del ejecutable 0.20 sigue siendo el log visual. No hay nuevo EXE.
No se oculta el mapa ni se anuncia una camara operativa sin imagen secundaria.

## Evidencia nueva del EXE

Inspeccion estatica con objdump del objeto de codigo situado en 0x88254,
direcciones virtuales a partir de 0x10000 (mismo archivo de versiones previas).

- El hook de actualizacion 0x37241 llama a 0x69ec9, que llama a 0x71026 y
  pone a FF el byte de datos 0xb3ffe. No demuestra una pasada de escena.
- La ruta alternativa 0x69ed6 lee 0x147b4, obtiene tres bytes desde una tabla
  y llama a 0x713ec y 0x2cbff. Tambien requiere clasificacion antes de reuso.
- 0x66080 guarda algunas variables, pero escribe numerosas globales:
  0xc4a18, 0xb404, 0x3aa20, 0xbf6b0..0xbf6b6, 0x3a734, entre otras.
  Indexa una tabla en 0x19fdc mediante AX y comprueba un tipo 0x8002.
  Tiene multiples llamadores (0x55917, 0x559f3, 0x67625, 0x67c90,
  0x695e7, entre otros). Hipotesis: es una rutina de dibujo de objetos,
  no una entrada demostrada para renderizar toda una camara.
- Los hooks 0x717c9 y de cockpit siguen siendo puntos de copia/composicion.

Las direcciones de datos anteriores son relativas al segmento del EXE;
no son direcciones absolutas del proceso RetroArch.

## Bloqueos concretos

1. Identificar la entrada de escena completa y su estado modificable. No
   invocar candidatos arbitrarios desde el hook del HUD.
2. Obtener un intervalo fiable: la torre actual usa distancia/vuelta anterior
   y media de hasta 30 muestras a 100 ms. No usar esa media para el umbral de
   0,2 s. El adaptador del controlador aun no esta implementado.
3. RESUELTO: Open Watcom restaurado en ../toolchains/watcom desde el snapshot
   oficial Current-build. Compilacion limpia del proyecto aprobada en
   validation/compiler-restore; f1battle.c tambien compila como objeto DOS.
4. Prueba instrumentada en juego para validar aislamiento y rendimiento.

La captura del usuario identifica DOSBox-Pure 1.0-preview4, Windows 64 bits.
Las configuraciones retroarch.cfg y DOSBox-pure.opt figuran como archivos
aportados en la conversacion, pero no se han inspeccionado en esta etapa.

## Siguiente prueba necesaria

Una vez localizada la entrada, registrar antes/despues de una pasada normal
el foco, modo de camara, viewport, destinos de dibujo y referencias globales.
Solo entonces construir una pasada secundaria aislada, verificar que no
avance la simulacion y medir su coste. Viabilidad aun no demostrada.
