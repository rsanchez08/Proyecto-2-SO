# Proyecto-2-SO

# 🧪 ¿Cómo correr el programa BWFS?

## ✅ 1. Compilar los módulos

### 📁 Compilar `mkfs`

```bash
cd mkfs
make
```

### 📁 Compilar `fsck`

```bash
cd ../fsck
make
```

### 📁 Compilar `mount`

```bash
cd ../mount
make
```

---

## ✅ 2. Crear el sistema de archivos

Desde la carpeta `mkfs`:

```bash
./mkfs output/
```

✔️ Esto crea el archivo `output/FS_0.png` que simula el disco del FS.

---

## ✅ 3. Verificar la integridad del FS

Desde la carpeta `fsck`:

```bash
./fsck ../mkfs/output/
```

✔️ Esto te dirá si el sistema de archivos está válido o dañado.

---

## ✅ 4. Montar el sistema de archivos

Desde la carpeta `mount`:

```bash
mkdir -p ../mnt       # crea carpeta vacía como punto de montaje
./mount ../mkfs/output ../mnt
```

✔️ Esto **monta  FS** en la carpeta `mnt/`.

---

## ✅ 5. Usar el sistema de archivos

Desde otra terminal o en la misma:

```bash
cd ../mnt
touch hola.txt
echo "Hola mundo" > hola.txt
cat hola.txt
ls -l
rm hola.txt
```

Todo esto se maneja por funciones `create`, `write`, `read`, `unlink`, etc.

---

## ✅ 6. Desmontar el FS

Cuando termines de probar:

```bash
fusermount3 -u ../mnt
```

✅ Esto libera el punto de montaje.
