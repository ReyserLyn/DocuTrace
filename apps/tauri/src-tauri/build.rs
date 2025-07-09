fn main() {
    // Cargar variables de entorno desde archivo .env
    if let Ok(env_path) = std::env::var("CARGO_MANIFEST_DIR") {
        let env_file = std::path::Path::new(&env_path)
            .parent()
            .unwrap()
            .join(".env");
        if env_file.exists() {
            println!("cargo:rerun-if-changed={}", env_file.display());

            if let Ok(content) = std::fs::read_to_string(&env_file) {
                for line in content.lines() {
                    if let Some((key, value)) = line.split_once('=') {
                        if key.trim().starts_with("TAURI_") {
                            println!("cargo:rustc-env={}={}", key.trim(), value.trim());
                        }
                    }
                }
            }
        }
    }

    tauri_build::build()
}
