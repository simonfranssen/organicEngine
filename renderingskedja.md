# Renderingskedjan

Detta dokument beskriver de olika stegen i renderingskedjan, från initialisering av SDL till att GPU:n renderar den första triangeln.

## 1. SDL och fönster

- Initiera SDL.
- Skapa ett fönster för rendering.

## 2. GPU-enhet

- Skapa en GPU Device.
- Koppla GPU-enheten till SDL-fönstret.

## 3. Huvudloop och kommandobuffert

- Implementera huvudloopen.
- Hämta en GPU Command Buffer för varje bildruta.
- Förbereda kommandon som ska skickas till GPU:n.

## 4. Swapchain och Render Target

- Hämta aktuell swapchain-textur.
- Definiera hur render passet ska använda bildytan.
- Förbereda render target för rendering.

## 5. Shaders och Pipeline

- Skapa vertex- och fragment-shaders.
- Konfigurera en Graphics Pipeline.
- Definiera hur geometrin ska bearbetas och renderas.

## 6. Draw Call

- Starta render passet.
- Binda Graphics Pipeline.
- Skicka en Draw Call till GPU:n.
- Rendera den första triangeln på skärmen.

## Slutmål

Renderingskedjan ska kunna rendera en triangel direkt i SDL-fönstret med hjälp av GPU:n.