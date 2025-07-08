// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use tauri_plugin_shell::ShellExt;
use tauri_plugin_shell::process::CommandEvent;

// Comando para verificar el estado del backend
#[tauri::command]
async fn check_backend_status() -> Result<String, String> {
    match reqwest::get("http://localhost:8000/health").await {
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
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![check_backend_status])
        .setup(|app| {
            // Iniciar el backend como sidecar
            let app_handle = app.handle().clone();
            
            tauri::async_runtime::spawn(async move {
                println!("[+] Iniciando backend DocuTrace...");
                
                let sidecar_command = app_handle.shell().sidecar("docutrace-backend").unwrap();
                let (mut rx, _child) = sidecar_command
                    .spawn()
                    .expect("Failed to spawn sidecar");
                
                println!("[+] Backend iniciado correctamente");
                
                // read events such as stdout
                while let Some(event) = rx.recv().await {
                    match event {
                        CommandEvent::Stdout(line_bytes) => {
                            let line = String::from_utf8_lossy(&line_bytes);
                            println!("backend stdout: {}", line);
                        }
                        CommandEvent::Stderr(line_bytes) => {
                            let line = String::from_utf8_lossy(&line_bytes);
                            eprintln!("backend stderr: {}", line);
                        }
                        CommandEvent::Error(error) => {
                            eprintln!("backend error: {}", error);
                        }
                        CommandEvent::Terminated(payload) => {
                            println!("backend terminated with code: {:?}", payload.code);
                            break;
                        }
                        _ => {}
                    }
                }
                
                println!("[+] Backend process finished");
            });
            
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

fn main() {
    run();
}
