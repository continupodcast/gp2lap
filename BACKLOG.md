# GP2Lap — backlog de la versión 0.8
Actualizado: 31 de octubre de 2026.

## Cierre
Versión 0.8 consolidada desde T20.7, con limpieza de material de integración retirado.
Última release de GitHub verificada por API: v0.56, 10/10/2026, commit 6a17be66dfa89a2e08fedc95656d1c588a569e20.
Referencia: https://github.com/continupodcast/gp2lap/releases/tag/v0.56
El paquete 0.8 está preparado; no se ha publicado ni creado un tag remoto.
Hecho significa implementación incluida. Las verificaciones aún pendientes se identifican por separado.

## Hecho en 0.8
- [x] Sistema opcional de compuestos por auto, identificación H/M/S y compuesto de clasificación separado.
- [x] Ensayos de grip, desgaste, combustible y tiempos; modelo de estrategia basado en las pruebas largas de A/B/C.
- [x] Optimización de estrategias por distancia, consumo y coste de boxes; hasta diez alternativas.
- [x] Familias de una, dos y tres paradas; variedad real de longitudes, ventanas escalonadas y distribución aleatoria para NPC.
- [x] Ponderación heurística por posición de salida y semilla propia reproducible/temporal del reparto de estrategias.
- [x] Aplicación del calendario, combustible inicial y compuestos; servicio también durante Accelerate Time.
- [x] Gestión de entradas imprevistas y cancelación de paradas futuras cuando el jugador elige cero.
- [x] Prioridad de un compuesto alternativo elegido manualmente; obligación de cambiar configurable.
- [x] Pantalla previa Race strategy editable, Default nativo, alternativas sin ranking ni delta y combustible recalculado.
- [x] Menú con fuente pixel suministrada, paleta próxima a GP2, relieves, botones individuales y navegación espacial.
- [x] Confirmación con Enter/espacio, relieve de presión y botones Restore Default / Ok / Cancel.
- [x] Nombre y mapa de la pista cargada; barra de stints H/M/S.
- [x] Selector Tyre integrado en boxes sin pantalla modal ni detención adicional del servicio.
- [x] Corrección de textos superpuestos, S visible y eliminación inmediata de corchetes al abandonar Stops to go.
- [x] Editor: sección Tyre Compound System y Require a compound change; CFG y proyectos compatibles.
- [x] Editor reorganizado y grilla Quickrace con colores, búsqueda, arrastre y edición de posiciones.
- [x] Grilla Quickrace personalizada opcional, sin modificar los participantes elegidos por GP2.
- [x] Clasificación combinada viernes/sábado implementada (pruebas finales independientes pendientes).
- [x] Controles adicionales de cámara opcionales integrados (no confundir con una segunda ventana de vídeo).
- [x] Eliminación de scripts/reportes de merge retirados y diagnósticos de compuestos inactivos; un único README de instalación.
- [x] Licencias y componentes activos de compatibilidad conservados.

## Pendiente y esfuerzo
Escala: bajo = cambio o comprobación localizada; medio = varios componentes y pruebas; alto = estado persistente o ingeniería inversa; muy alto = investigación con factibilidad incierta.
Estimaciones relativas, no compromisos de tiempo.

| Pendiente | Esfuerzo | Estado / alcance |
|---|---|---|
| Comprobar en GP2 la última opción Require a compound change ON/OFF y la limpieza de 0.8 | Bajo | Pruebas controladas y compilación aprobadas; falta confirmación del usuario para la última opción. |
| Actualizar el manual público a 0.8 y preparar publicación en GitHub | Bajo–medio | Paquete y notas listos; sin publicación remota. |
| Completar matriz de pruebas Quickrace / fin de semana / campeonato y clasificación combinada | Medio | Integración disponible; no consta validación exhaustiva de todos los modos. |
| Conservar estrategia, historial de compuestos y progreso al guardar/cargar | Alto | No implementado; las partidas guardadas no conservan actualmente el estado nuevo. |
| Varios pilotos humanos con estrategias independientes | Alto | Actualmente un único piloto humano. |
| Calibración por pista de consumo, pérdida en boxes y degradación | Alto | Curvas actuales de un circuito y coste de referencia de 27 s; no hay calibración universal automática. |
| Incorporar longitud, características de curvas y potencia relativa del motor al modelo | Alto | Roadmap acordado; aún no son variables calibradas del optimizador. |
| Tráfico, ritmo relativo y posición de reincorporación para adaptar la estrategia | Muy alto | Sólo heurística de parrilla actual; no hay simulación validada de tráfico. |
| Reoptimizar automáticamente el resto de carrera tras accidentes o cambios manuales | Alto | Se aplica la decisión local; no se resuelve de nuevo todo el futuro. |
| Random Seed del generador original de GP2 al comenzar un evento | Alto | Pendiente; distinto de StrategyAutoSeed, que ya controla el reparto propio. |
| Safety Car | Muy alto | Factibilidad pendiente; no incluido. |

## En pausa o descartado
- Vuelcos completos: en pausa por decisión previa, sin solución validada.
- Segunda ventana de batallas y animación externa de ruedas: descartadas; no reabrir como pendientes activos.
- Persistencia combinada de microsectores viernes/sábado: fuera de alcance; limitación de Accelerate Time documentada.

## Funciones de v0.56 conservadas
Cámaras externas en boxes, tarjetas de tiempos de boxes, mapas y zoom, sectores/microsectores, temas moderno/90s, controles de mensajes, replay, bandera por piloto, logos y fuel drain. No se cuentan como novedades de 0.8.

## Registro histórico anterior
Lo siguiente se conserva por trazabilidad; sus estados quedan sustituidos por las secciones anteriores.

# GP2Lap — backlog actualizado al 12 de octubre de 2026

## Base estable
- v0.56: base R2 validada por el usuario y elegida para el release de GitHub. Notas consolidadas en README. El usuario realizó el merge y preparó el release; publicación final no confirmada explícitamente.

## En evaluación
- v0.60: clasificación combinada viernes/sábado mediante TAB con la torre de qualy activa. Vista del día por defecto; funciona con ambos temas y TV/onboard. Lee las tablas originales por CarId y el mejor tiempo del día para actualizar en vivo. No modifica parrilla ni tablas nativas. Compilada y con pruebas de cálculo; validación en juego pendiente. F1QUAL.LOG registra las entradas al pulsar TAB.

## Pendientes, de menor a mayor esfuerzo estimado
1. Manual de usuario actualizado: controles, editor, cámaras externas en boxes, pit timing, límites de Accelerate Time y, tras validación, tabla combinada v0.60.
2. Random Seed configurable al comenzar un evento nuevo: localizar la función original de estado de cinco bytes, avanzar con temporizador DOS antes de rendimientos/parrilla/fallos, preservar continuidad del fin de semana, cargas y repeticiones. Investigación pendiente.
3. Safety Car basado en GP2Mem: detectar autos detenidos en pista y evaluar intervención segura en carrera. Factibilidad pendiente.

## En pausa
- Compuestos (experimento anterior, excluido de la nueva v0.60): columna en torre moderna/90s y TV/onboard. A -> H gris claro, B -> M amarillo, C -> S rojo, D -> S violeta, letra menor. Lectura actual pCarSetups[CarId-1].compound (0..3), todavía no acreditada como compuesto montado. Prueba visual funcional informada. Paquete rremedio gp2lap-e-2 revisado: PerCarTyreCompounds por estrategia, SoftTyresInQuali D y ShowTyreCompoundBadge; primera opción desactivada por defecto. Ejecutable comprimido examinado, sin fuente ni override.cfg. No es merge directo; requiere fuentes o reimplementación y validar estado efectivo, agarre/desgaste y cambios en pits. Equivalencias H/M/S son presentación, no conversión de física.
- Vuelcos completos: investigación suspendida por el usuario tras D6. Identificados límite angular y contribuciones de ruedas que invierten rotación; sin solución validada. Fuentes y logs D1-D6/P1 conservados, excluidos de versiones estables.

## Validado / resuelto
- Cámaras externas en boxes para jugador e IA, opción AllowExternalPitCamera en CFG y editor.
- Tarjetas de tiempo de boxes: tecla 4 en carrera, solo TV, apiladas, mensajes ON/OFF, resultado congelado 5 segundos.
- Mapa completo: color original de pista, punto seleccionado del color del equipo 50% mayor y sobre línea central, autos mejor posicionados por encima. v0.56 R2 validada.
- Optimización del mapa completo con zooms y mapa local.
- Tarjetas de sectores TV centrada abajo y onboard compacta arriba; teclas 4/5 excluyentes en práctica/clasificación.
- Colores personales/absolutos y actualización retroactiva de récords para sectores/microsectores.
- Accelerate Time conserva sectores originales S1/S2/vuelta y registros previos; no registra nuevos microsectores durante aceleración.
- Controles individuales de anuncios nativos; vuelta rápida textual oculta; caché de texto corregida y Processor Occupancy visible.
- Torre oculta por defecto en replay, tecla 3 permite reactivarla.
- Bandera a cuadros por piloto que termina la carrera.
- Logo de equipo eliminado de tarjeta de sectores 90s.
- Material de presentación: video, checklist y textos para YouTube/Discord/phpBB preparados.

## Cancelado / descartado
- Ventana secundaria para batallas.
- Animación de giro de ruedas externas.
- Persistencia de sectores/microsectores de viernes a sábado fuera del alcance de la nueva tabla combinada de vueltas.

