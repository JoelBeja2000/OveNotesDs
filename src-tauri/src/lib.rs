use std::fs;
use std::io::Read;
use std::net::UdpSocket;
use std::path::PathBuf;
use std::thread;
use tauri::Manager;
use tiny_http::{Method, Response, Server};

// Helper function to get local IP address
fn get_local_ip() -> Option<String> {
    let socket = UdpSocket::bind("0.0.0.0:0").ok()?;
    socket.connect("8.8.8.8:80").ok()?;
    socket.local_addr().ok().map(|addr| addr.ip().to_string())
}

// Helper to encode IP + Port into a Base36 pairing code
fn encode_pairing_code(ip: &str, port: u16) -> String {
    let parts: Vec<u8> = ip.split('.').filter_map(|s| s.parse::<u8>().ok()).collect();
    if parts.len() != 4 {
        return "0000000000".to_string();
    }
    let mut value: u64 = 0;
    for i in 0..4 {
        value = (value << 8) | parts[i] as u64;
    }
    value = (value << 16) | port as u64;

    let chars = b"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    let mut code = String::new();
    let mut val = value;
    while val > 0 {
        let rem = (val % 36) as usize;
        code.insert(0, chars[rem] as char);
        val /= 36;
    }
    while code.len() < 10 {
        code.insert(0, '0');
    }
    code
}

// Helper to add CORS headers to tiny_http responses
fn add_cors_headers<R: Read>(response: Response<R>) -> Response<R> {
    response
        .with_header(
            tiny_http::Header::from_bytes(&b"Access-Control-Allow-Origin"[..], &b"*"[..]).unwrap(),
        )
        .with_header(
            tiny_http::Header::from_bytes(
                &b"Access-Control-Allow-Methods"[..],
                &b"GET, POST, OPTIONS, DELETE"[..],
            )
            .unwrap(),
        )
        .with_header(
            tiny_http::Header::from_bytes(
                &b"Access-Control-Allow-Headers"[..],
                &b"Content-Type"[..],
            )
            .unwrap(),
        )
}

// Start the background HTTP/Notes server in a dedicated thread
fn start_rust_server(app_handle: tauri::AppHandle, notes_dir: PathBuf) {
    thread::spawn(move || {
        let server =
            Server::http("0.0.0.0:3000").expect("Failed to start HTTP server on port 3000");
        let _ = fs::create_dir_all(&notes_dir);

        for mut request in server.incoming_requests() {
            let url = request.url().to_string();
            let method = request.method().clone();

            if method == Method::Options {
                let response = add_cors_headers(Response::empty(204));
                let _ = request.respond(response);
                continue;
            }

            if url == "/api/pairing-code" && method == Method::Get {
                let ip = get_local_ip().unwrap_or_else(|| "127.0.0.1".to_string());
                let port = 3000;
                let code = encode_pairing_code(&ip, port);
                let json = format!(r#"{{"code":"{}","ip":"{}","port":{}}}"#, code, ip, port);
                let response = add_cors_headers(
                    Response::from_string(json)
                        .with_status_code(200)
                        .with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                );
                let _ = request.respond(response);
            } else if url == "/api/notas" && method == Method::Get {
                let mut notes = Vec::new();
                if let Ok(entries) = fs::read_dir(&notes_dir) {
                    for entry in entries.filter_map(Result::ok) {
                        let path = entry.path();
                        if path.is_file() && path.extension().map_or(false, |ext| ext == "png") {
                            if let Some(name) = path.file_name().and_then(|n| n.to_str()) {
                                if let Ok(metadata) = entry.metadata() {
                                    let time = metadata
                                        .modified()
                                        .ok()
                                        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
                                        .map_or(0.0, |d| d.as_millis() as f64);
                                    notes.push((name.to_string(), time));
                                }
                            }
                        }
                    }
                }
                notes.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));

                let mut json = String::from("[");
                for (i, (name, time)) in notes.iter().enumerate() {
                    if i > 0 {
                        json.push(',');
                    }
                    json.push_str(&format!(r#"{{"name":"{}","time":{}}}"#, name, time));
                }
                json.push(']');

                let response = add_cors_headers(
                    Response::from_string(json)
                        .with_status_code(200)
                        .with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                );
                let _ = request.respond(response);
            } else if url.starts_with("/api/notas/") && method == Method::Delete {
                let filename = url.trim_start_matches("/api/notas/");
                let file_path = notes_dir.join(filename);
                let success = if file_path.exists() && fs::remove_file(file_path).is_ok() {
                    r#"{"success":true}"#
                } else {
                    r#"{"success":false,"error":"File not found or cannot delete"}"#
                };
                let response = add_cors_headers(
                    Response::from_string(success)
                        .with_status_code(200)
                        .with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                );
                let _ = request.respond(response);
            } else if url == "/api/nueva-nota" && method == Method::Post {
                let mut body = Vec::new();
                let _ = request.as_reader().read_to_end(&mut body);

                let now = std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .map(|d| d.as_millis())
                    .unwrap_or(0);
                let filename = format!("nota_{}.png", now);
                let file_path = notes_dir.join(&filename);

                let success = if fs::write(&file_path, body).is_ok() {
                    format!(r#"{{"success":true,"filename":"{}"}}"#, filename)
                } else {
                    r#"{"success":false,"error":"Could not save file"}"#.to_string()
                };

                let response = add_cors_headers(
                    Response::from_string(success)
                        .with_status_code(201)
                        .with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                );
                let _ = request.respond(response);
            } else if url.starts_with("/notas/") && method == Method::Get {
                let parts: Vec<&str> = url.split('?').collect();
                let path_part = parts[0];
                let is_download = parts.get(1).map_or(false, |q| q.contains("download=1"));
                let filename = path_part.trim_start_matches("/notas/");
                let decoded_filename = percent_encoding::percent_decode_str(filename)
                    .decode_utf8_lossy()
                    .into_owned();
                let file_path = notes_dir.join(&decoded_filename);
                if file_path.exists() {
                    if let Ok(file_data) = fs::read(&file_path) {
                        let mut response = add_cors_headers(
                            Response::from_data(file_data)
                                .with_status_code(200)
                                .with_header(
                                    tiny_http::Header::from_bytes(
                                        &b"Content-Type"[..],
                                        &b"image/png"[..],
                                    )
                                    .unwrap(),
                                ),
                        );
                        if is_download {
                            let disposition = format!("attachment; filename=\"{}\"", decoded_filename);
                            response = response.with_header(
                                tiny_http::Header::from_bytes(
                                    &b"Content-Disposition"[..],
                                    disposition.as_bytes(),
                                )
                                .unwrap()
                            );
                        }
                        let _ = request.respond(response);
                        continue;
                    }
                }
                let response =
                    add_cors_headers(Response::from_string("Not Found").with_status_code(404));
                let _ = request.respond(response);
            } else if url == "/" || url == "/index.html" {
                if let Ok(mut html) = fs::read_to_string("dist/index.html") {
                    let ip = get_local_ip().unwrap_or_else(|| "127.0.0.1".to_string());
                    let port = 3000;
                    let code = encode_pairing_code(&ip, port);

                    let injection_token =
                        "let pairingCodeData = { code: '------', ip: '...', port: '...' };";
                    let injected = format!(
                        r#"let pairingCodeData = {{ code: '{}', ip: '{}', port: {} }};"#,
                        code, ip, port
                    );
                    html = html.replace(injection_token, &injected);

                    let mut notes = Vec::new();
                    if let Ok(entries) = fs::read_dir(&notes_dir) {
                        for entry in entries.filter_map(Result::ok) {
                            let path = entry.path();
                            if path.is_file() && path.extension().map_or(false, |ext| ext == "png")
                            {
                                if let Some(name) = path.file_name().and_then(|n| n.to_str()) {
                                    if let Ok(metadata) = entry.metadata() {
                                        let time = metadata
                                            .modified()
                                            .ok()
                                            .and_then(|t| {
                                                t.duration_since(std::time::UNIX_EPOCH).ok()
                                            })
                                            .map_or(0.0, |d| d.as_millis() as f64);
                                        notes.push((name.to_string(), time));
                                    }
                                }
                            }
                        }
                    }
                    notes
                        .sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));

                    let mut cached_notes_json = String::from("[");
                    for (i, (name, time)) in notes.iter().enumerate() {
                        if i > 0 {
                            cached_notes_json.push(',');
                        }
                        cached_notes_json
                            .push_str(&format!(r#"{{"name":"{}","time":{}}}"#, name, time));
                    }
                    cached_notes_json.push(']');

                    let cached_token = "let cachedNotes = [];";
                    let cached_injected = format!("let cachedNotes = {};", cached_notes_json);
                    html = html.replace(cached_token, &cached_injected);

                    let response = add_cors_headers(
                        Response::from_string(html)
                            .with_status_code(200)
                            .with_header(
                                tiny_http::Header::from_bytes(
                                    &b"Content-Type"[..],
                                    &b"text/html; charset=utf-8"[..],
                                )
                                .unwrap(),
                            ),
                    );
                    let _ = request.respond(response);
                } else {
                    let response = add_cors_headers(
                        Response::from_string("index.html not found").with_status_code(404),
                    );
                    let _ = request.respond(response);
                }
            } else if url == "/manifest.json" {
                if let Ok(data) = fs::read("dist/manifest.json") {
                    let response = add_cors_headers(
                        Response::from_data(data).with_status_code(200).with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                    );
                    let _ = request.respond(response);
                } else {
                    let response = add_cors_headers(Response::empty(404));
                    let _ = request.respond(response);
                }
            } else if url == "/logo_pwa.png" {
                if let Ok(data) = fs::read("dist/logo_pwa.png") {
                    let response = add_cors_headers(
                        Response::from_data(data).with_status_code(200).with_header(
                            tiny_http::Header::from_bytes(&b"Content-Type"[..], &b"image/png"[..])
                                .unwrap(),
                        ),
                    );
                    let _ = request.respond(response);
                } else {
                    let response = add_cors_headers(Response::empty(404));
                    let _ = request.respond(response);
                }
            } else if url == "/sw.js" {
                if let Ok(data) = fs::read("dist/sw.js") {
                    let response = add_cors_headers(
                        Response::from_data(data).with_status_code(200).with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/javascript"[..],
                            )
                            .unwrap(),
                        ),
                    );
                    let _ = request.respond(response);
                } else {
                    let response = add_cors_headers(Response::empty(404));
                    let _ = request.respond(response);
                }
            } else if url == "/api/restart" && method == Method::Post {
                let response = add_cors_headers(
                    Response::from_string(r#"{"success":true,"message":"Server is restarting..."}"#)
                        .with_status_code(200)
                        .with_header(
                            tiny_http::Header::from_bytes(
                                &b"Content-Type"[..],
                                &b"application/json"[..],
                            )
                            .unwrap(),
                        ),
                );
                let _ = request.respond(response);

                let app_c = app_handle.clone();
                thread::spawn(move || {
                    thread::sleep(std::time::Duration::from_millis(500));
                    app_c.restart();
                });
            } else {
                let response =
                    add_cors_headers(Response::from_string("Not Found").with_status_code(404));
                let _ = request.respond(response);
            }
        }
    });
}

#[tauri::command]
fn save_note_file(app: tauri::AppHandle, filename: String) -> Result<(), String> {
    use tauri_plugin_dialog::DialogExt;

    let notes_dir = if cfg!(target_os = "android") {
        app.path()
            .app_local_data_dir()
            .unwrap_or_else(|_| std::path::PathBuf::from("."))
            .join("Notas_Publicadas")
    } else {
        std::path::PathBuf::from("Notas_Publicadas")
    };

    let source_path = notes_dir.join(&filename);
    if !source_path.exists() {
        return Err(format!("File {} not found", filename));
    }

    let file_path = app
        .dialog()
        .file()
        .set_file_name(&filename)
        .add_filter("Image", &["png"])
        .blocking_save_file();

    if let Some(dest_path) = file_path {
        if let Ok(dest_path_buf) = dest_path.into_path() {
            if let Err(e) = std::fs::copy(&source_path, &dest_path_buf) {
                return Err(format!("Failed to copy file: {}", e));
            }
            Ok(())
        } else {
            Err("Invalid destination path".to_string())
        }
    } else {
        Err("User cancelled download".to_string())
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_process::init())
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![save_note_file])
        .setup(|app| {
            let notes_dir = if cfg!(target_os = "android") {
                app.path()
                    .app_local_data_dir()
                    .unwrap_or_else(|_| std::path::PathBuf::from("."))
                    .join("Notas_Publicadas")
            } else {
                std::path::PathBuf::from("Notas_Publicadas")
            };

            start_rust_server(app.handle().clone(), notes_dir);

            if cfg!(debug_assertions) {
                app.handle().plugin(
                    tauri_plugin_log::Builder::default()
                        .level(log::LevelFilter::Info)
                        .build(),
                )?;
            }
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
