// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

#[tauri::command]
async fn check_backend_status(app: tauri::AppHandle) -> Result<String, String> {
    let client = app.reqwest_client();
    
    let backend_url = option_env!("TAURI_BACKEND_URL")
        .unwrap_or("http://localhost:8000");
    let health_url = format!("{}/health", backend_url);
    
    match client.get(&health_url).send().await {
        Ok(response) => {
            if response.status().is_success() {
                Ok("Backend conectado".to_string())
            } else {
                Err("Backend no responde correctamente".to_string())
            }
        }
        Err(_) => Err("No se puede conectar al backend".to_string()),
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![check_backend_status])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

fn main() {
    run();
}
