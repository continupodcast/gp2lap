# Entrega: branches modulares de GP2Lap F1 HUD

La separación está implementada en **18 branches nuevas: 16 módulos, una base común y una integración completa**, sobre el commit publicado de v0.8. La referencia estable y el tag v0.8 se conservaron.

Las branches están terminadas en el repositorio transferible. **No están publicadas en GitHub**: el intento de acceso de escritura devolvió `could not read Username ... terminal prompts disabled`; esta sesión no dispone de autenticación para publicar.

## Validación

- 21 compilaciones DOS desde cero: core, 16 branches de módulos, integración y tres combinaciones adicionales.
- 14 controles de lógica aprobados: compuestos, reglas opcionales, estrategias, menú previo, navegación de boxes, grilla, editor y módulos gráficos omitidos.
- Dependencias y archivos exactos comprobados en las 16 branches de módulos.
- Todas las branches de módulos son ancestros de la integración; sus merges finalizaron sin conflictos.
- La reconstrucción integrada después de otros perfiles produjo el mismo SHA-256: `cd6ed7925ed9ee27718539ac2c700eac08e3e40c01b5ebc8fa4615d978bcd876`.
- No se ejecutó GP2: queda pendiente la validación dentro del juego, especialmente guardado/carga, aceleración, menús y cámaras.

## Archivos del paquete

- `GP2Lap-v0.8-modular.bundle`: repositorio Git con las branches, tags e historial.
- `GP2Lap-Modular-Integrated-Source.zip`: código de la integración para inspección; el ZIP de código no conserva branches.
- `builds/`: EXE y CFG de cada variante, capacidades, hashes y logs. Elegir una sola variante para copiar al juego.
- `validation/`: matriz completa y resultados de las pruebas.
- `MODULARITY.md`: dependencias, alcance, instrucciones de compilación y límites.
- `import_modular_bundle.py`: crea una copia local nueva para abrirla en GitHub Desktop, sin sobrescribir el repositorio existente y sin publicar.

## Abrir en GitHub Desktop

1. Extraer este paquete en una carpeta.
2. Desde esa carpeta, ejecutar `python import_modular_bundle.py ../GP2Lap-Modular` (o `python3` en macOS/Linux). Requiere Python 3 y Git; también busca el Git incluido en GitHub Desktop.
3. En GitHub Desktop, usar **File → Add Local Repository** y seleccionar la nueva carpeta `GP2Lap-Modular`.
4. La branch inicial es `modular/integrated`; las 18 branches nuevas están disponibles localmente. Publicarlas requerirá la sesión autenticada de GitHub Desktop.

El script configura como remoto el repositorio de continupodcast, pero no hace push ni crea una release. Para importación manual también se puede clonar el bundle con Git.

## Alcance de la modularidad

Los módulos se combinan al compilar y sus dependencias viajan con cada branch. Las entradas de funciones están extraídas a archivos propios; el core conserva la ABI C/assembler, estructuras del juego, modelos auxiliares, telemetría compartida y primitivas de dibujo. No es un cargador de plugins dinámicos ni son parches independientes para una base distinta. Las extensiones heredadas del GP2Lap de rremedio permanecen en la plataforma y conservan su atribución.

La configuración sigue siendo opcional. Un CFG no puede activar una capacidad omitida de la compilación. Para seleccionar otros subconjuntos se usa `tools/modules.py`; la compilación fuerza la reconstrucción de todos los objetos.

## Matriz de compilación

| Branch o combinación | Tamaño EXE | Resultado |
| --- | ---: | --- |
| `modular/core` | 825212 bytes | PASS |
| `modules/timing-tower` | 840428 bytes | PASS |
| `modules/driver-panels` | 839768 bytes | PASS |
| `modules/sectors` | 840912 bytes | PASS |
| `modules/circuit-maps` | 839844 bytes | PASS |
| `modules/pit-timing` | 835108 bytes | PASS |
| `modules/native-captions` | 829716 bytes | PASS |
| `modules/camera-controls` | 839248 bytes | PASS |
| `modules/fuel-drain` | 829824 bytes | PASS |
| `modules/quickrace-grid` | 835240 bytes | PASS |
| `modules/tyre-compounds` | 829604 bytes | PASS |
| `modules/tyre-compound-hud` | 841012 bytes | PASS |
| `modules/race-strategy-engine` | 836500 bytes | PASS |
| `modules/player-race-strategy` | 851276 bytes | PASS |
| `modules/pit-compound-selector` | 851868 bytes | PASS |
| `modules/combined-qualifying` | 840428 bytes | PASS |
| `modules/hud-editor` | 825212 bytes | PASS |
| `modular/integrated` | 912808 bytes | PASS |
| `maps-and-pit-selector` | 862376 bytes | PASS |
| `tower-sectors-and-cameras` | 860532 bytes | PASS |
| `compounds-without-strategies` | 841012 bytes | PASS |

Los detalles y hashes completos están en `validation/build-matrix.json`. Los warnings por parámetros sin uso en stubs deshabilitados pertenecen a la compatibilidad de ABI; no hay errores de compilación o enlace.
