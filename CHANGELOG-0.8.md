# GP2Lap F1 HUD 0.8 — cambios desde GitHub v0.56

Base comprobada por API y tag: v0.56, publicada el 10/10/2026, commit `6a17be66dfa89a2e08fedc95656d1c588a569e20`.
https://github.com/continupodcast/gp2lap/releases/tag/v0.56

0.8 consolida T20.7 y retira material de experimentación que ya no se usa. No crea una publicación ni un tag remoto.

## Compuestos y física medida
- Compuestos por auto y etiquetas H/M/S en el HUD; selección separada para clasificación.
- Ensayos cortos y largos de A/B/C: grip, desgaste, combustible y tiempos. D se reserva para clasificación.
- Modelo propio de tiempos y corrección por carga de combustible a partir de los ensayos largos.
- Aplicación de cambios de compuesto a través del servicio de boxes, también al acelerar el tiempo.
- Sistema opcional, con controles unificados en Tyre Compound System.

## Estrategias NPC
- Cálculo de alternativas según vueltas, costes de neumáticos, combustible y boxes (referencia actual: 27 segundos).
- Familias de una, dos y tres paradas, con hasta diez alternativas; cero paradas admitidas en Quickrace.
- Reparto aleatorio con semilla temporal o fija. Cuotas por familias, heurística de parrilla y separación de ventanas.
- Variedad de longitudes de stints en lugar de sólo invertir el orden de compuestos.
- Calendario y combustible inicial aplicados por auto. La semilla propia no cambia el generador original de GP2.
- Caché y reutilización de cálculos para reducir la espera del selector.

## Estrategia del jugador
- Pantalla Race strategy con la estrategia nativa seleccionada como Default.
- Alternativas numeradas y aleatorizadas; sin revelar ranking, deltas ni estimaciones comparativas.
- Edición de número de paradas, vueltas de parada y compuestos; combustible inicial calculado automáticamente.
- Barra de stints y nombre/mapa real del circuito cargado.
- Estética similar a GP2: paleta, fuente pixel suministrada, bevel, esquinas, botones pequeños −/+ y presión individual.
- Flechas según la distribución visual; Enter o espacio para activar. Restore Default / Ok / Cancel en una fila.

## Boxes durante la carrera
- Tyre integrado en el menú nativo junto a Fuel laps y Stops to go. Sin selector modal extra.
- Navegación nativa y servicio continuo, incluso tras reparaciones.
- Selección manual de un compuesto diferente al programado; prioridad de la decisión del jugador.
- Cero paradas futuras cancela el calendario restante. Si corresponde, se respeta la regla pendiente de compuesto.
- Correcciones de superposición de textos, S ausente y foco/corchetes que persistían en otra fila.
- Require a compound change permite desactivar la obligación; la parada obligatoria sigue siendo una regla independiente.

## Editor y grilla
- Sección única Tyre Compound System y opciones de estrategias, distribución, selección del jugador, boxes y obligación de compuesto.
- Organización más compacta del editor y persistencia de preferencias en proyectos.
- Grilla Quickrace con tarjetas por color de equipo, búsqueda, arrastre, botones y posición directa.
- Orden personalizado de los participantes ya elegidos por GP2; no añade autos ni modifica clasificaciones de fin de semana.
- Migración/importación de CFG anteriores y exportación que desactiva los antiguos overrides de ensayo y diagnóstico.

## Otras incorporaciones respecto de v0.56
- Tabla combinada de clasificación viernes/sábado, con soporte de temas y vistas. La validación exhaustiva de modos permanece pendiente.
- Controles adicionales de cámara, seguimiento y órbita opcionales; no equivalen a una segunda ventana de vídeo.
- Infraestructura integrada de compatibilidad para recursos y configuración por auto/equipo: texturas, cascos, formas, datos de pilotos, parrilla, física, boxes, sesiones y calendario. Se conserva porque sigue enlazada y llamada por el programa; no se elimina por su procedencia. Su presencia no acredita pruebas exhaustivas de cada opción de terceros.

## Limpieza del cierre
- Se retiraron scripts y reportes del merge experimental que no participan en la compilación ni en el uso actual.
- Se eliminaron el diagnóstico de compuestos desactivado y sus llamadas; la resolución efectiva del compuesto se conserva en f1compoundstate.h.
- Se quitaron comentarios de integración experimental de las plantillas del editor/CFG.
- Un README de instalación; backlog y cambios separados. Avisos de licencia conservados.
- Identificación del ejecutable y del editor actualizada a 0.8.

## Límites y estado
- Un solo piloto humano; SVGA 640×480.
- Sin persistencia del nuevo estado de estrategias al guardar/cargar.
- Sin calibración automática universal por pista, tráfico simulado ni potencia relativa en el optimizador.
- GP2 original conserva el control de carga de combustible en boxes. Un cambio manual modifica el stint actual, no reoptimiza toda la carrera.
- Compilación y regresiones controladas aprobadas. El usuario confirmó el funcionamiento del selector y sus iteraciones; la última opción de obligación de compuesto y la limpieza de 0.8 aún requieren comprobación dentro de GP2.

## Auditoría de fuentes
Se incluye `validation/source-diff-v056-v08.json` y el listado de abajo. Compara src, inc, tools, tests, f1-input y makefiles con el tag; excluye objetos, ejecutables y otros resultados de compilación. No representa un PR publicado.

### Archivos nuevos (70)

- `f1-input/fonts/PIXEARG_.TTF`
- `src/aiaccel.c`
- `src/aiaccel.h`
- `src/aibank.c`
- `src/aibank.h`
- `src/calendar.c`
- `src/calendar.h`
- `src/carhelmet.c`
- `src/carhelmet.h`
- `src/carshape.c`
- `src/carshape.h`
- `src/cartex.c`
- `src/cartex.h`
- `src/drvdata.c`
- `src/drvdata.h`
- `src/f1autostrategy.h`
- `src/f1camtest.h`
- `src/f1combined.h`
- `src/f1compoundrule.h`
- `src/f1compoundstate.h`
- `src/f1csv.h`
- `src/f1features.h`
- `src/f1grid90.inc`
- `src/f1inlinepit.h`
- `src/f1playermodal.h`
- `src/f1playeroptions.h`
- `src/f1playerpalette.h`
- `src/f1playershuffle.h`
- `src/f1playerstrategy.h`
- `src/f1playerui.h`
- `src/f1quickgrid.h`
- `src/f1simulationmode.h`
- `src/f1strategycalc.h`
- `src/f1strategydistribution.h`
- `src/f1strategyedit.h`
- `src/f1strategyfont.h`
- `src/f1strategymodel.h`
- `src/f1strategynav.h`
- `src/f1strategyservice.h`
- `src/f1strategytest.h`
- `src/f1tyreage.h`
- `src/f1tyrefuel.h`
- `src/f1tyrelaps.h`
- `src/f1tyrenativelaps.h`
- `src/f1tyres.h`
- `src/f1tyretest.h`
- `src/gridcap.c`
- `src/gridcap.h`
- `src/jamidcap.c`
- `src/jamidcap.h`
- `src/magicd.c`
- `src/magicd.h`
- `src/override.c`
- `src/override.h`
- `src/pitcrew.c`
- `src/pitcrew.h`
- `src/pitstops.c`
- `src/pitstops.h`
- `src/roster.c`
- `src/roster.h`
- `src/sessions.c`
- `src/sessions.h`
- `src/svga/f1gridprobe.inc`
- `src/teamphys.c`
- `src/teamphys.h`
- `tests/test_override.c`
- `tools/build_strategy_font.py`
- `tools/gp2edit2override.py`
- `tools/test_combined.c`
- `tools/test_grid_menu.py`

### Archivos modificados (30)

- `makefile`
- `makefile.lin`
- `src/cfgdefin.inc`
- `src/f1advanced.c`
- `src/f1gapdiag.h`
- `src/f1grid.inc`
- `src/f1loops.h`
- `src/f1pitcamera.h`
- `src/f1qualy.c`
- `src/f1qualy.h`
- `src/f1r90.inc`
- `src/f1race.c`
- `src/f1render.c`
- `src/f1render.h`
- `src/f1timing.c`
- `src/f1tower.c`
- `src/frankasm.inc`
- `src/gp2hook.c`
- `src/gp2lap.c`
- `src/int9.c`
- `src/keyhand.c`
- `src/lammcall.asm`
- `src/svga/gp2pics.c`
- `src/timing/codehook.inc`
- `src/timing/convert.inc`
- `src/timing/qatl.inc`
- `src/timing/rbehind.c`
- `src/timing/rlaptime.c`
- `src/timing/timinstd.h`
- `tools/editor.html`

### Archivos retirados respecto de v0.56 (0)

Ninguno.
