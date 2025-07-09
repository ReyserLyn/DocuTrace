import wikipedia
import os

# Configura el idioma a español
wikipedia.set_lang("es")

# Lista de temas variados y realistas
temas = [
    "Inteligencia artificial", "Ciencia de datos", "Historia de Perú", "Arequipa", "Machu Picchu",
    "Isaac Newton", "Albert Einstein", "Literatura española", "Pedro Castillo", "Universidad Nacional de San Agustín",
    "Geometría", "Física cuántica", "Química", "Lengua española", "Matemáticas", "Lógica matemática",
    "Cultura Inca", "Amazonas", "Pablo Neruda", "Gabriel García Márquez", "Teoría de la relatividad",
    "Economía", "Sociología", "Psicología", "Software libre", "GNU/Linux", "Python", "C++", "Bases de datos",
    "Árbol AVL", "Índice invertido", "Google", "Historia de Internet", "Ciberseguridad", "Criptografía",
    "Blockchain", "Inteligencia emocional", "Educación en Perú", "Literatura latinoamericana", "Mario Vargas Llosa",
    "Astronomía", "Astrofísica", "Planetas", "Energía solar", "Cambio climático", "Nanotecnología", "Biotecnología",
    "Tsunami", "Terremoto", "Volcán Misti", "Fenómeno El Niño", "Virreinato del Perú", "Simón Bolívar",
    "José de San Martín", "Independencia del Perú", "Túpac Amaru II", "Revolución Industrial", "Segunda Guerra Mundial",
    "Imperio Romano", "Imperio Inca", "Cuzco", "Trujillo", "Amazonía peruana", "Lago Titicaca", "Costa del Perú",
    "Democracia", "Dictadura", "Gobierno del Perú", "Congreso del Perú", "Constitución del Perú", "Derechos Humanos",
    "Naciones Unidas", "IA generativa", "Aprendizaje automático", "Red neuronal artificial", "Algoritmos",
    "Big Data", "Cómputo cuántico", "Redes sociales", "WhatsApp", "TikTok", "Facebook", "Google Translate",
    "Buscadores web", "Videojuegos", "Industria musical", "Cine peruano", "Festival de Cannes", "Premios Nobel",
    "Wikipedia", "OpenAI", "ChatGPT", "Tecnología educativa", "Innovación tecnológica", "Startups en Perú",
    "Historia de Arequipa", "Puente Chilina", "Yanahuara", "Cañón del Colca", "Mistura", "RPP Noticias"
]

# Crear carpeta si no existe
os.makedirs("docs_es", exist_ok=True)

fallidos = []

for i, tema in enumerate(temas):
    try:
        contenido = wikipedia.page(tema).content
        filename = f"docs_es/doc_{i:03}_{tema.replace(' ', '_')}.txt"
        with open(filename, "w", encoding="utf-8") as f:
            f.write(contenido)
        print(f"[✓] Guardado: {filename}")
    except Exception as e:
        print(f"[✗] Fallo en: {tema} -> {e}")
        fallidos.append(tema)

print("\n--- RESUMEN ---")
print(f"Generados correctamente: {len(temas) - len(fallidos)}")
print(f"Fallidos: {len(fallidos)}")
if fallidos:
    print("Temas fallidos:", fallidos)

