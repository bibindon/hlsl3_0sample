#pragma comment( lib, "d3d9.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#include <fstream>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPD3DXFONT g_pFont = NULL;
LPD3DXMESH pMesh = NULL;
D3DMATERIAL9* pMaterials = NULL;
LPDIRECT3DTEXTURE9* pTextures = NULL;
DWORD dwNumMaterials = 0;
D3DXMATERIAL* d3dxMaterials = NULL;
float f = 0.0f;
LPDIRECT3DVERTEXSHADER9 pVertexShader;
LPDIRECT3DPIXELSHADER9 pPixelShader;
D3DXHANDLE hWorldViewProj;
LPD3DXCONSTANTTABLE pConstantTable = NULL; // 定数テーブル

void TextDraw(LPD3DXFONT pFont, char* text, int X, int Y)
{
    RECT rect = { X,Y,0,0 };
    pFont->DrawText(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 0, 0, 0));
}

HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        return E_FAIL;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Flags = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
    {
        if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
        {
            return(E_FAIL);
        }
    }

    HRESULT hr = D3DXCreateFont(
        g_pd3dDevice,
        20,
        0,
        FW_HEAVY,
        1,
        false,
        SHIFTJIS_CHARSET,
        OUT_TT_ONLY_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE,
        "ＭＳ ゴシック",
        &g_pFont);
    if FAILED(hr)
    {
        return(E_FAIL);
    }

    LPD3DXBUFFER pD3DXMtrlBuffer = NULL;

    if (FAILED(D3DXLoadMeshFromX("cube.x", D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, NULL, &pD3DXMtrlBuffer, NULL,
        &dwNumMaterials, &pMesh)))
    {
        MessageBox(NULL, "Xファイルの読み込みに失敗しました", NULL, MB_OK);
        return E_FAIL;
    }

    DWORD* wordBuffer { static_cast<DWORD*>(pD3DXMtrlBuffer->GetBufferPointer()) };

    D3DXComputeNormals(pMesh, wordBuffer);

    d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    pMaterials = new D3DMATERIAL9[dwNumMaterials];
    pTextures = new LPDIRECT3DTEXTURE9[dwNumMaterials];

    for (DWORD i = 0; i < dwNumMaterials; i++)
    {
        pMaterials[i] = d3dxMaterials[i].MatD3D;
        pMaterials[i].Ambient = pMaterials[i].Diffuse;
        pTextures[i] = NULL;
        if (d3dxMaterials[i].pTextureFilename != NULL &&
            lstrlen(d3dxMaterials[i].pTextureFilename) > 0)
        {
            if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice,
                d3dxMaterials[i].pTextureFilename,
                &pTextures[i])))
            {
                MessageBox(NULL, "テクスチャの読み込みに失敗しました", NULL, MB_OK);
            }
        }
    }
    pD3DXMtrlBuffer->Release();

    LPDIRECT3DDEVICE9 pDevice = g_pd3dDevice; // 初期化済みの DirectX デバイス

    {
        LPD3DXBUFFER pCode = NULL;             // コンパイルされたシェーダーコード
        LPD3DXBUFFER pErrorMsgs = NULL;        // エラーメッセージ

        HRESULT hr = D3DXCompileShaderFromFile(
            "vs1.hlsl",     // シェーダーソースファイル
            NULL,                     // マクロ定義
            NULL,                     // 定数の定義
            "main",                   // エントリーポイント（関数名）
            "vs_3_0",                 // シェーダーモデル
            0,                        // コンパイルフラグ
            &pCode,                   // コンパイルされたコード
            &pErrorMsgs,              // エラーメッセージ用バッファ
            &pConstantTable           // 定数テーブル
        );
        hr = pDevice->CreateVertexShader(
            (DWORD*)pCode->GetBufferPointer(),  // コンパイルされたシェーダーコード
            &pVertexShader                      // 作成された頂点シェーダーオブジェクト
        );
    }
    {
        LPD3DXBUFFER pPixelCode;    // コンパイルされたピクセルシェーダーコード
        LPD3DXBUFFER pErrorMsgs = NULL;        // エラーメッセージ

        hr = D3DXCompileShaderFromFile(
            "ps1.hlsl",      // ピクセルシェーダーファイル名
            NULL,                   // 定数の定義
            NULL,                   // マクロ定義
            "main",                 // シェーダーのエントリポイント
            "ps_3_0",               // シェーダーモデル
            0,                      // コンパイルフラグ
            &pPixelCode,            // コンパイルされたコード
            &pErrorMsgs,            // エラー出力用バッファ
            NULL                    // ピクセルシェーダーの場合、定数テーブルは不要
        );

        hr = pDevice->CreatePixelShader(
            (DWORD*)pPixelCode->GetBufferPointer(),  // コンパイル済みコード
            &pPixelShader                            // 作成されたピクセルシェーダーオブジェクト
        );

    }

    pDevice->SetVertexShader(pVertexShader);
    pDevice->SetPixelShader(pPixelShader);

    hWorldViewProj = pConstantTable->GetConstantByName(NULL, "matWorldViewProj");

    pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR); // ミニフィルタリング（縮小時）
    pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR); // マグニフィルタリング（拡大時）
    pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); // ミップマップのフィルタリング
    pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP); // U 座標のアドレッシングモード
    pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP); // V 座標のアドレッシングモード

    return S_OK;
}

VOID Cleanup()
{
    SAFE_RELEASE(pMesh);
    SAFE_RELEASE(g_pFont);
    SAFE_RELEASE(g_pd3dDevice);
    SAFE_RELEASE(g_pD3D);
}

VOID Render()
{
    if (NULL == g_pd3dDevice)
    {
        return;
    }
    f += 0.025f;

    D3DXMATRIX mat;
    D3DXMATRIX View, Proj;
    D3DXMatrixPerspectiveFovLH(&Proj, D3DXToRadian(45), 640.0f / 480.0f, 1.0f, 10000.0f);
    D3DXVECTOR3 vec1(10 * sinf(f), 10, -10 * cosf(f));
    D3DXVECTOR3 vec2(0, 0, 0);
    D3DXVECTOR3 vec3(0, 1, 0);
    D3DXMatrixLookAtLH(&View, &vec1, &vec2, &vec3);
    D3DXMatrixIdentity(&mat);
    mat = mat * View * Proj;
    pConstantTable->SetMatrix(g_pd3dDevice, hWorldViewProj, &mat);

    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_XRGB(100, 100, 100), 1.0f, 0);

    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        char msg[100];
        strcpy_s(msg, 100, "Xファイルの読み込みと表示");
        TextDraw(g_pFont, msg, 0, 0);

        for (DWORD i = 0; i < dwNumMaterials; i++)
        {
            g_pd3dDevice->SetTexture(0, pTextures[i]);
            pMesh->DrawSubset(i);
        }
        g_pd3dDevice->EndScene();
    }

    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        Render();
        return 0;
    case WM_SIZE:
        InvalidateRect(hWnd, NULL, true);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ INT)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "Window1", NULL };
    RegisterClassEx(&wc);

    RECT rect;
    SetRect(&rect, 0, 0, 640, 480);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    HWND hWnd = CreateWindow("Window1", "Hello DirectX9 World !!",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
        NULL, NULL, wc.hInstance, NULL);

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnregisterClass("Window1", wc.hInstance);
    return 0;
}
