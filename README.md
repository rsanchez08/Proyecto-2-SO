# Proyecto-2-SO

## 🖼️ Vista del sistema de archivos

# Superbloque y bitmap

![photo_2025-06-30 21 16 21](https://github.com/user-attachments/assets/bd9544d8-6114-4be3-8e13-5e66a9308c69)

*Imagen generada por mkfs.bwfs que muestra el superbloque y el mapa de bits en blanco y negro.*

# Estructura montada del FS

![photo_2025-06-30 21 16 18](https://github.com/user-attachments/assets/cd927853-5910-4876-8160-c30ef2de337a)

*Sistema montado con archivos visibles desde el directorio `mnt/`.*

# 🧪 ¿Cómo correr el programa BWFS?

## ✅ 1. Compilar los módulos

### 📁 Compilar `mkfs`

```bash
cd mkfs
make
```

### 📁 Compilar `fsck`

```bash
cd fsck
make
```

### 📁 Compilar `mount`

```bash
cd mount
make
```

---

## ✅ 2. Crear el sistema de archivos

Desde la carpeta `mkfs`:

```bash
./mkfs.bwfs output/
```

✔️ Esto crea el archivo `output/FS_0.png` que simula el disco del FS.

---

## ✅ 3. Verificar la integridad del FS

Desde la carpeta `fsck`:

```bash
./fsck.bwfs ../mkfs/output/
```

✔️ Esto te dirá si el sistema de archivos está válido o dañado.

---

## ✅ 4. Montar el sistema de archivos

Desde la carpeta `mount`:

```bash
mkdir -p ../mnt       # crea carpeta vacía como punto de montaje
./mount.bwfs ../mkfs/output ../mnt
```

✔️ Esto **monta  FS** en la carpeta `mnt/`.

---

## ✅ 5. Usar el sistema de archivos

Desde otra terminal o en la misma:

```bash
cd mnt
echo "Hola mundo" > hola.txt
cat hola.txt
ls -l
rm hola.txt
```

Todo esto se maneja por funciones `create`, `write`, `read`, `unlink`, etc.

---

## ✅ 6. Desmontar el FS

Cuando termines de probar:
Asegurarse que el FS no tenga tareas corriendo. 
```bash
lsof +D mnt/
```
```bash
kill -9 <PID>
```
Correr desde carpeta root.
```bash
fusermount3 -u mnt/
```

✅ Esto libera el punto de montaje.
