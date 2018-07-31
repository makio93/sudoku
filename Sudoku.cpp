// ver.5.3.1
// メッセージの細々したバグを修正

#include <windows.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include "resource.h"

#define N 9
#define N2 3
#define SQUARE_SIZE 50
#define YOHAKU 25
#define BUTTON_WIDTH 100
#define WAIT_TIME 1000
#define ONE_WAIT_TIME 10
#define TIME_UNIT 1000000
#define RANGE 5.0
#define TARGET_MIN 1
#define TARGET_MAX 10
#define MESSAGE_MAX 10000
#define ID_BUTTON_START 101
#define ID_BUTTON_DEL 100
#define ID_BUTTON_ANALYZE 0
#define ID_BUTTON_GENERATE 1
#define ID_BUTTON_SOLVE 2
#define ID_EDIT 3
#define MD_ANALYZE 1
#define MD_GENERATE 2
#define MD_SOLVE 3
#define SMD_BIG 1
#define SMD_SMALL 2
#define CB_END 0
#define CB_FINISH 1
#define CB_DOUBLE 2
#define CB_NIL 3
#define CB_TIMEOUT 4
#define CB_ERROR 5
#define CB_HIGH 6
#define MB_ERROR -1
#define MB_IMPOSSIBLE -2
#define MB_TIMEOUT -3
#define MB_ZERO -4
#define FIB_ERROR -1
#define FIB_NULL INT_MAX
#define FIB_DEFAULT -10
#define DB_ERROR -1
#define DB_DEFAULT -10

typedef struct {
	int x, y;
}BANPOS;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
void MessageAdd(HWND, LPTSTR);
void WINAPI Analyze();
void WINAPI Generate();
void WINAPI Solve();
void Try();
BOOL FindBrank(int*, int*);
int isOkeru(int, int, int);
BOOL CheckSolvable();
int CheckBan(HWND);
void ChangeMode(HMENU, WORD, MENUITEMINFO*);
BOOL CALLBACK MyDlgProc(HWND, UINT, WPARAM, LPARAM);
int MakeBan();
void ExitSolveThCheck();
void BeforeThreadEnd();
BOOL StartGenerate();
BOOL EndTryCheck(BOOL*, BOOL*);
void ChangeSizeMode(HMENU, MENUITEMINFO*);
int MakeBan();
int FillInBan(BOOL);
int DrillBan();
void MenuSwitch(BOOL);
BOOL DisToBanana();

TCHAR szClassName[] = TEXT("Sudoku"), add[64], szLevel[64]; // ウィンドウクラス
HINSTANCE hInst;
int Display[N][N], BanAnalyze[N][N], SubAnalyze[N][N], Stock[N][N], SmallDisplay[N][N][N2][N2], 
	hardness = 0, mode, sizemode, target, TryCount, Time, TotalTime = 0, CountUp = 0;
BOOL ExRem = FALSE, DSolution = FALSE, isSolving = FALSE, TimeOut = FALSE, isRunMA = FALSE, isReady = FALSE, 
	CanEscape = FALSE, ThEnd = TRUE, mbisfirst = TRUE, isInput[N][N], notarget = FALSE, TotalTimeOut = FALSE, isDrilling = FALSE;
RECT BanPaint = { YOHAKU, YOHAKU, YOHAKU + SQUARE_SIZE * N, YOHAKU + SQUARE_SIZE * N };
HGLOBAL hStrMem = NULL;
LPTSTR lpszBuf = NULL;
HWND hEdit, hButton[N + 4], hMainWnd;
HMENU hMenu;

// Windowsプログラムのエントリーポイント
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL bRet;

	HINSTANCE hInst = hCurInst;

	if (!InitApp(hCurInst))
		return FALSE;
	if (!InitInstance(hCurInst, nCmdShow))
		return FALSE;

	// メッセージを取得
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		}
		else {
			TranslateMessage(&msg); // メッセージを変換
			DispatchMessage(&msg); // メッセージを送出
		}
	}
	return (int)msg.wParam;
}

// ウィンドウクラスの登録
ATOM InitApp(HINSTANCE hInst)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX); // 構造体のサイズ
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // クラスのスタイル
	wc.lpfnWndProc = WndProc; // プロシージャ名
	wc.cbClsExtra = 0; // 補助メモリ
	wc.cbWndExtra = 0; // 補助メモリ
	wc.hInstance = hInst; // インスタンス
	wc.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(MYICON), 
		IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); // アイコン
	wc.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE((WORD)IDC_ARROW), 
		IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED); // カーソル
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH); // 背景ブラシ
	wc.lpszMenuName = TEXT("MYMENU"); // メニュー名
	wc.lpszClassName = szClassName; // クラス名
	wc.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(MYICON), 
		IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED); // 小さいアイコン

	return (RegisterClassEx(&wc));
}

// ウィンドウの生成
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	RECT rc;

	rc.left = 0;
	rc.right = BMPW;
	rc.top = 0;
	rc.bottom = BMPH;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME, TRUE);
	hMainWnd = CreateWindow(szClassName, // クラス名
		TEXT("数独解け～る"), // ウィンドウ名
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME, // ウィンドウスタイル
		CW_USEDEFAULT, // x位置
		CW_USEDEFAULT, // y位置
		rc.right - rc.left, // ウィンドウ幅
		rc.bottom - rc.top, // ウィンドウ高さ
		NULL, // 親ウィンドウのハンドル、親を作るときはNULL
		NULL, // メニューハンドル、クラスメニューを使うときはNULL
		hInst, // インスタンスハンドル
		NULL // ウィンドウ作成データ
	);

	if (!hMainWnd)
		return FALSE;
	ShowWindow(hMainWnd, nCmdShow); // ウィンドウの表示状態を設定
	UpdateWindow(hMainWnd); // ウィンドウを更新

	return TRUE;
}

// ウィンドウプロシージャ（コールバック関数）
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int i, j, k, l, ButtonPosX, ButtonPosY;
	static BANPOS CurPos = { 0, 0 }, SCurPos = { 0, 0 };
	POINTS CurPosPx;
	HDC hdc;
	PAINTSTRUCT ps;
	static HPEN hBanPen[3], hNullPen;
	static HBRUSH hBanBrush, hColorBrush, hInputBrush, hNullBrush;
	static HFONT hBanFont, hInputFont, hSmallBanFont, hEditFont;
	static HANDLE hSolveTh = NULL;
	static MENUITEMINFO mii;
	TCHAR StrNum[9][2] = { TEXT("１"), TEXT("２"), TEXT("３"), TEXT("４"),
		TEXT("５"), TEXT("６"), TEXT("７"), TEXT("８"), TEXT("９"), };

	switch (msg) {
	case WM_CREATE:
		memset(BanAnalyze, 0, sizeof(int) * N * N);
		memset(Display, 0, sizeof(int) * N * N);
		memset(Stock, 0, sizeof(int) * N * N);
		memset(SubAnalyze, 0, sizeof(int) * N * N);
		memset(isInput, (BOOL)FALSE, sizeof(BOOL) * N * N);

		hBanPen[0] = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
		hBanPen[1] = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
		hBanPen[2] = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		hNullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
		hColorBrush = (HBRUSH)GetStockObject(DC_BRUSH);
		hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		hBanFont = CreateFont(SQUARE_SIZE, SQUARE_SIZE / 2, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, FIXED_PITCH | FF_DECORATIVE, NULL);
		hInputFont = CreateFont(SQUARE_SIZE, SQUARE_SIZE / 2, 0, 0,
			FW_BOLD, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, FIXED_PITCH | FF_DECORATIVE, NULL);
		hSmallBanFont = CreateFont(SQUARE_SIZE / N2, SQUARE_SIZE / N2 / 2, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			ANTIALIASED_QUALITY, FIXED_PITCH | FF_DECORATIVE, NULL);
		hEditFont = CreateFont(SQUARE_SIZE * 3 / 10, SQUARE_SIZE * 3 / 30, 0, 0,
			FW_DONTCARE, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, NULL);

		ButtonPosX = YOHAKU * 2 + SQUARE_SIZE * N + BUTTON_WIDTH;
		ButtonPosY = YOHAKU + SQUARE_SIZE * ((N - 1) - (N + 1) / 2) - SQUARE_SIZE;
		hEdit = CreateWindow(TEXT("EDIT"), TEXT(""),
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
			ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_READONLY,
			ButtonPosX - BUTTON_WIDTH, YOHAKU, BUTTON_WIDTH * 2,
			(ButtonPosY + SQUARE_SIZE) - YOHAKU, hWnd, (HMENU)ID_EDIT, hInst, NULL);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hEditFont, MAKELPARAM(FALSE, 0));
		for (i = 1; i <= N; i++) {
			if (i % 2 == 0)
				ButtonPosX += BUTTON_WIDTH;
			else {
				ButtonPosX -= BUTTON_WIDTH;
				ButtonPosY += SQUARE_SIZE;
			}
			hButton[i] = CreateWindow(TEXT("BUTTON"),
				StrNum[i - 1], WS_CHILD | WS_VISIBLE,
				ButtonPosX, ButtonPosY, 100, SQUARE_SIZE,
				hWnd, (HMENU)(ID_BUTTON_START + (i - 1)), hInst, NULL);
		}
		ButtonPosX += BUTTON_WIDTH;
		hButton[0] = CreateWindow(TEXT("BUTTON"),
			TEXT("DEL"), WS_CHILD | WS_VISIBLE,
			ButtonPosX, ButtonPosY, 100, SQUARE_SIZE,
			hWnd, (HMENU)ID_BUTTON_DEL, hInst, NULL);
		ButtonPosX -= BUTTON_WIDTH;
		ButtonPosY += SQUARE_SIZE;
		hButton[N + 1] = CreateWindow(TEXT("BUTTON"),
			TEXT("解析！"), WS_CHILD,
			ButtonPosX, ButtonPosY, 200, SQUARE_SIZE,
			hWnd, (HMENU)ID_BUTTON_ANALYZE, hInst, NULL);
		hButton[N + 2] = CreateWindow(TEXT("BUTTON"),
			TEXT("問題生成"), WS_CHILD,
			ButtonPosX, ButtonPosY, 200, SQUARE_SIZE,
			hWnd, (HMENU)ID_BUTTON_GENERATE, hInst, NULL);
		hButton[N + 3] = CreateWindow(TEXT("BUTTON"),
			TEXT("答え合わせ"), WS_CHILD,
			ButtonPosX, ButtonPosY, 200, SQUARE_SIZE,
			hWnd, (HMENU)ID_BUTTON_SOLVE, hInst, NULL);

		hMenu = GetMenu(hWnd);

		mode = MD_ANALYZE;
		sizemode = SMD_BIG;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_STRING;

		MessageAdd(hEdit, TEXT("初期化終了…\r\n"));

		ChangeMode(hMenu, IDM_ANALYZE, &mii);

		break;
	case WM_COMMAND:
		switch (sizemode) {
		case SMD_BIG:
			if (!(mode == MD_SOLVE && isInput[CurPos.y][CurPos.x])) {
				if ((ID_BUTTON_START <= LOWORD(wp) && LOWORD(wp) < ID_BUTTON_START + N)) {
					Display[CurPos.y][CurPos.x] = LOWORD(wp) - ID_BUTTON_START + 1;
					if (mode != MD_SOLVE)
						isInput[CurPos.y][CurPos.x] = TRUE;
				}
				else if (LOWORD(wp) == ID_BUTTON_DEL) {
					Display[CurPos.y][CurPos.x] = 0;
					isInput[CurPos.y][CurPos.x] = FALSE;
				}
				InvalidateRect(hWnd, &BanPaint, TRUE);
			}
			SetFocus(hWnd);
			break;
		case SMD_SMALL:
			if (mode == MD_SOLVE) {
				if (!isInput[CurPos.y][CurPos.x]) {
					if ((ID_BUTTON_START <= LOWORD(wp) && LOWORD(wp) < ID_BUTTON_START + N) || 
						LOWORD(wp) == ID_BUTTON_DEL) {
						SmallDisplay[CurPos.y][CurPos.x][SCurPos.y][SCurPos.x] = LOWORD(wp) - ID_BUTTON_START + 1;
						InvalidateRect(hWnd, &BanPaint, TRUE);
					}
				}
			}
			SetFocus(hWnd);
			break;
		}

		switch (LOWORD(wp)) {
		case IDM_ANALYZE:
		case IDM_GENERATE:
		case IDM_SOLVE:
			ChangeMode(hMenu, LOWORD(wp), &mii);
			break;
		case ID_BUTTON_ANALYZE:
			if (!isSolving) {
				if (hSolveTh != NULL) {
					CloseHandle(hSolveTh);
					hSolveTh = NULL;
				}
				ThEnd = FALSE;
				hSolveTh = CreateThread(
					NULL, 0, (LPTHREAD_START_ROUTINE)Analyze,
					NULL, 0, NULL);
			}
			else {
				ThEnd = TRUE;
			}
			SetFocus(hWnd);
			break;
		case ID_BUTTON_GENERATE:
			if (!isSolving) {
				if (StartGenerate()) {
					if (hSolveTh != NULL) {
						CloseHandle(hSolveTh);
						hSolveTh = NULL;
					}
					ThEnd = FALSE;
					hSolveTh = CreateThread(
						NULL, 0, (LPTHREAD_START_ROUTINE)Generate,
						NULL, 0, NULL);
				}
			}
			else {
				ThEnd = TRUE;
			}
			SetFocus(hWnd);
			break;
		case ID_BUTTON_SOLVE:
			if (!isSolving) {
				if (hSolveTh != NULL) {
					CloseHandle(hSolveTh);
					hSolveTh = NULL;
				}
				ThEnd = FALSE;
				hSolveTh = CreateThread(
					NULL, 0, (LPTHREAD_START_ROUTINE)Solve,
					NULL, 0, NULL);
			}
			else {
				ThEnd = TRUE;
			}
			SetFocus(hWnd);
			break;
		case IDM_SAVEINPUT:
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					if (Display[i][j]) {
						isInput[i][j] = TRUE;
					}
				}
			}
			InvalidateRect(hWnd, &BanPaint, TRUE);
			MessageAdd(hEdit, TEXT("現在の盤面を全て入力データにしました。\r\n"));
			break;
		case IDM_OUTPUTDEL:
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					if (!isInput[i][j])
						Display[i][j] = 0;
				}
			}
			InvalidateRect(hWnd, &BanPaint, TRUE);
			if (mode != MD_SOLVE) {
				MessageAdd(hEdit, TEXT("出力結果を全て削除しました。\r\n"));
			}
			else {
				MessageAdd(hEdit, TEXT("解答を全て削除しました。\r\n"));
			}
			if (mode == MD_SOLVE)
				SendMessage(hMainWnd, WM_COMMAND, ID_BUTTON_SOLVE, 0);
			break;
		case IDM_SMALLDEL:
			memset(SmallDisplay, 0, sizeof(int) * N * N * N2 * N2);
			InvalidateRect(hWnd, &BanPaint, TRUE);
			MessageAdd(hEdit, TEXT("小文字を全て削除しました。\r\n"));
			break;
		case IDM_ALLDEL:
			memset(Display, 0, sizeof(int) * N * N);
			memset(isInput, (BOOL)FALSE, sizeof(BOOL) * N * N);
			InvalidateRect(hWnd, &BanPaint, TRUE);
			MessageAdd(hEdit, TEXT("盤面を全て削除しました。\r\n"));
			break;
		case IDM_SIZECHANGE:
			ChangeSizeMode(hMenu, &mii);
			break;
		case IDM_END:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if (!isSolving) {
			CurPosPx = MAKEPOINTS(lp);
			if (CurPosPx.x - YOHAKU >= 0 && CurPosPx.x - YOHAKU < N * SQUARE_SIZE &&
				CurPosPx.y - YOHAKU >= 0 && CurPosPx.y - YOHAKU < N * SQUARE_SIZE) {
				CurPos = { (int)((CurPosPx.x - YOHAKU) / (double)SQUARE_SIZE), 
					(int)((CurPosPx.y - YOHAKU) / (double)SQUARE_SIZE) };
				switch (sizemode) {
				case SMD_BIG:
					SCurPos = { 0, 0 };
					break;
				case SMD_SMALL:
					SCurPos = { (int)((CurPosPx.x - YOHAKU) / ((double)SQUARE_SIZE / N2)) % N2, 
						(int)((CurPosPx.y - YOHAKU) / ((double)SQUARE_SIZE / N2)) % N2 };
					break;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (!isSolving) {
			CurPosPx = MAKEPOINTS(lp);
			if (CurPosPx.x - YOHAKU >= 0 && CurPosPx.x - YOHAKU < N * SQUARE_SIZE &&
				CurPosPx.y - YOHAKU >= 0 && CurPosPx.y - YOHAKU < N * SQUARE_SIZE) {
				SendMessage(hWnd, WM_LBUTTONDOWN, wp, lp);
				ChangeSizeMode(hMenu, &mii);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case WM_KEYDOWN:
		if (!isSolving) {
			switch (wp) {
			case VK_BACK:
			case VK_DELETE:
				if (!(mode == MD_SOLVE && isInput[CurPos.y][CurPos.x])) {
					switch (sizemode) {
					case SMD_BIG:
						Display[CurPos.y][CurPos.x] = 0;
						isInput[CurPos.y][CurPos.x] = FALSE;
					break;
					case SMD_SMALL:
						SmallDisplay[CurPos.y][CurPos.x][SCurPos.y][SCurPos.x] = 0;
						break;
					}
				}
				break;
			case VK_RETURN:
			case VK_SEPARATOR:
				switch (mode) {
				case MD_ANALYZE:
					if (!isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_ANALYZE, 0);
					break;
				case MD_GENERATE:
					if (!isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_GENERATE, 0);
					break;
				case MD_SOLVE:
					if (!isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_SOLVE, 0);
					break;
				}
				break;
			case VK_LEFT:
			case VK_HOME:
				switch (sizemode) {
				case SMD_BIG:
					if (CurPos.x > 0)
						CurPos.x--;
					break;
				case SMD_SMALL:
					if (N2 * CurPos.x + SCurPos.x > 0) {
						if (SCurPos.x > 0)
							SCurPos.x--;
						else {
							SCurPos.x = (N2 - 1) - SCurPos.x;
							CurPos.x--;
						}
					}
					break;
				}
				break;
			case VK_UP:
			case VK_PRIOR:
				switch (sizemode) {
				case SMD_BIG:
					if (CurPos.y > 0)
						CurPos.y--;
					break;
				case SMD_SMALL:
					if (N2 * CurPos.y + SCurPos.y > 0) {
						if (SCurPos.y > 0)
							SCurPos.y--;
						else {
							SCurPos.y = (N2 - 1) - SCurPos.y;
							CurPos.y--;
						}
					}
					break;
				}
				break;
			case VK_RIGHT:
			case VK_END:
				switch (sizemode) {
				case SMD_BIG:
					if (CurPos.x < N - 1)
						CurPos.x++;
					break;
				case SMD_SMALL:
					if (N2 * CurPos.x + SCurPos.x < N * N2 - 1) {
						if (SCurPos.x < N2 - 1)
							SCurPos.x++;
						else {
							SCurPos.x = (N2 - 1) - SCurPos.x;
							CurPos.x++;
						}
					}
					break;
				}
				break;
			case VK_DOWN:
			case VK_NEXT:
				switch (sizemode) {
				case SMD_BIG:
					if (CurPos.y < N - 1)
						CurPos.y++;
					break;
				case SMD_SMALL:
					if (N2 * CurPos.y + SCurPos.y < N * N2 - 1) {
						if (SCurPos.y < N2 - 1)
							SCurPos.y++;
						else {
							SCurPos.y = (N2 - 1) - SCurPos.y;
							CurPos.y++;
						}
					}
					break;
				}
				break;
			}
		}
		else {
			switch (wp) {
			case VK_ESCAPE:
				switch (mode) {
				case MD_ANALYZE:
					if (isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_ANALYZE, 0);
					break;
				case MD_GENERATE:
					if (isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_GENERATE, 0);
					break;
				case MD_SOLVE:
					if (isSolving)
						SendMessage(hWnd, WM_COMMAND, ID_BUTTON_SOLVE, 0);
					break;
				}
				break;
			}
		}
		InvalidateRect(hWnd, &BanPaint, TRUE);
		break;
	case WM_CHAR:
		if (!isSolving) {
			switch (sizemode) {
			case SMD_BIG:
				if (!(mode == MD_SOLVE && isInput[CurPos.y][CurPos.x])) {
					if ('1' <= wp && wp <= '9') {
						Display[CurPos.y][CurPos.x] = wp - '0';
						if (mode != MD_SOLVE)
							isInput[CurPos.y][CurPos.x] = TRUE;
					}
					else if (VK_NUMPAD1 <= wp && wp <= VK_NUMPAD9) {
						Display[CurPos.y][CurPos.x] = wp - VK_NUMPAD0;
						if (mode != MD_SOLVE)
							isInput[CurPos.y][CurPos.x] = TRUE;
					}
				}
				break;
			case SMD_SMALL:
				if (!(mode == MD_SOLVE && isInput[CurPos.y][CurPos.x])) {
					if ('1' <= wp && wp <= '9') {
						SmallDisplay[CurPos.y][CurPos.x][SCurPos.y][SCurPos.x] = wp - '0';
					}
					else if (VK_NUMPAD1 <= wp && wp <= VK_NUMPAD9) {
						SmallDisplay[CurPos.y][CurPos.x][SCurPos.y][SCurPos.x] = wp - VK_NUMPAD0;
					}
				}
				break;
			}
		}
		InvalidateRect(hWnd, &BanPaint, TRUE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		SelectObject(hdc, hNullPen);
		if (!isSolving && !isReady)
			hBanBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		else
			hBanBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		SelectObject(hdc, hBanBrush);
		Rectangle(hdc, YOHAKU, YOHAKU, YOHAKU + SQUARE_SIZE * N, YOHAKU + SQUARE_SIZE * N);

		if (!isSolving && !isReady) {
			SelectObject(hdc, hColorBrush);
			SetDCBrushColor(hdc, RGB(255, 255, 127));
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					if (isInput[i][j]) {
						Rectangle(hdc, YOHAKU + j * SQUARE_SIZE, YOHAKU + i * SQUARE_SIZE,
							YOHAKU + (j + 1) * SQUARE_SIZE + 1, YOHAKU + (i + 1) * SQUARE_SIZE + 1);
					}
				}
			}

			if (isInput[CurPos.y][CurPos.x])
				SetDCBrushColor(hdc, RGB(207, 207, 223));
			else
				SetDCBrushColor(hdc, RGB(127, 127, 255));
			switch (sizemode) {
			case SMD_BIG:
				Rectangle(hdc, YOHAKU + CurPos.x * SQUARE_SIZE, YOHAKU + CurPos.y * SQUARE_SIZE,
					YOHAKU + (CurPos.x + 1) * SQUARE_SIZE + 1, YOHAKU + (CurPos.y + 1) * SQUARE_SIZE + 1);
				break;
			case SMD_SMALL:
				Rectangle(hdc, YOHAKU + CurPos.x * SQUARE_SIZE + SCurPos.x * (SQUARE_SIZE / N2), 
					YOHAKU + CurPos.y * SQUARE_SIZE + SCurPos.y * (SQUARE_SIZE / N2), 
					YOHAKU + CurPos.x * SQUARE_SIZE + (int)ceil((SCurPos.x + 1) * ((double)SQUARE_SIZE / N2)) + 1,
					YOHAKU + CurPos.y * SQUARE_SIZE + (int)ceil((SCurPos.y + 1) * ((double)SQUARE_SIZE / N2)) + 1);
				break;
			}
		}

		SelectObject(hdc, hBanPen[0]);
		SelectObject(hdc, hNullBrush);
		Rectangle(hdc, YOHAKU, YOHAKU, YOHAKU + SQUARE_SIZE * N, YOHAKU + SQUARE_SIZE * N);

		SelectObject(hdc, hBanPen[2]);
		for (i = 1; i < N; i++) {
			if (i % 3 == 0)
				SelectObject(hdc, hBanPen[1]);
			MoveToEx(hdc, YOHAKU + i * SQUARE_SIZE, YOHAKU, NULL);
			LineTo(hdc, YOHAKU + i * SQUARE_SIZE, YOHAKU + N * SQUARE_SIZE);
			if (i % 3 == 0)
				SelectObject(hdc, hBanPen[2]);
		}

		for (i = 1; i < N; i++) {
			if (i % 3 == 0)
				SelectObject(hdc, hBanPen[1]);
			MoveToEx(hdc, YOHAKU, YOHAKU + i * SQUARE_SIZE, NULL);
			LineTo(hdc, YOHAKU + N * SQUARE_SIZE, YOHAKU + i * SQUARE_SIZE);
			if (i % 3 == 0)
				SelectObject(hdc, hBanPen[2]);
		}

		SetBkMode(hdc, TRANSPARENT);

		if (mode == MD_SOLVE) {
			SelectObject(hdc, hSmallBanFont);
			SetTextColor(hdc, RGB(0, 0, 0));
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					for (k = 0; k < N2; k++) {
						for (l = 0; l < N2; l++) {
							if (SmallDisplay[i][j][k][l]) {
								TextOut(hdc, YOHAKU + j * SQUARE_SIZE + l * SQUARE_SIZE / N2, 
									YOHAKU + i * SQUARE_SIZE + k * SQUARE_SIZE / N2,
									StrNum[SmallDisplay[i][j][k][l] - 1], lstrlen(StrNum[SmallDisplay[i][j][k][l] - 1]));
							}
						}
					}
				}
			}
		}

		SelectObject(hdc, hBanFont);
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (Display[i][j]) {
					if (!isInput[i][j]) {
						SelectObject(hdc, hBanFont);
						if (!isSolving && !isReady)
							SetTextColor(hdc, RGB(0, 0, 0));
						else {
							SetTextColor(hdc, RGB(127, 127, 127));
						}
					}
					else if (isInput[i][j]) {
						SelectObject(hdc, hInputFont);
						if (!isSolving && !isReady)
							SetTextColor(hdc, RGB(127, 0, 0));
						else
							SetTextColor(hdc, RGB(191, 127, 127));
					}

					if(!((isSolving || isReady) && !isInput[i][j]))
						TextOut(hdc, YOHAKU + j * SQUARE_SIZE, YOHAKU + i * SQUARE_SIZE,
							StrNum[Display[i][j] - 1], lstrlen(StrNum[Display[i][j] - 1]));
				}
			}
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		MessageAdd(hEdit, TEXT("プログラムを終了しています…\r\n"));
		ThEnd = TRUE;
		if (hSolveTh != NULL) {
			CloseHandle(hSolveTh);
			hSolveTh = NULL;
		}
		GlobalUnlock(hStrMem);
		GlobalFree(hStrMem);
		for (i = 0; i < 3; i++)
			DeleteObject(hBanPen[i]);
		DeleteObject(hNullPen);
		DeleteObject(hBanFont);
		DeleteObject(hInputFont);
		DeleteObject(hSmallBanFont);
		DeleteObject(hEditFont);
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return (DefWindowProc(hWnd, msg, wp, lp));
	}
	return 0;
}

BOOL StartGenerate() {
	int i;
	INT_PTR DialogButton;

	isReady = TRUE;
	SetWindowText(hButton[N + 2], TEXT("中止"));
	MessageAdd(hEdit, TEXT("問題生成開始…\r\n"));
	for (i = 0; i < N + 1; i++) {
		EnableWindow(hButton[i], FALSE);
	}
	InvalidateRect(hMainWnd, &BanPaint, TRUE);

	do {
		DialogButton = DialogBox(hInst, TEXT("MYDLG"), hMainWnd, (DLGPROC)MyDlgProc);
		switch (DialogButton) {
		case IDOK:
			if (!lstrcmp(szLevel, TEXT("n"))) {
				notarget = TRUE;
				isReady = FALSE;
				return TRUE;
			}
			else {
				target = _wtoi_l(szLevel, _get_current_locale());
				if (!(target >= TARGET_MIN && target <= TARGET_MAX)) {
					MessageAdd(hEdit, TEXT("入力値が正しくありません！\r\n"));
				}
				else {
					notarget = FALSE;
					isReady = FALSE;
					return TRUE;
				}
			}
			break;
		case IDCANCEL:
			for (i = 0; i < N + 1; i++) {
				EnableWindow(hButton[i], TRUE);
			}
			SetWindowText(hButton[11], TEXT("問題生成"));
			MessageAdd(hEdit, TEXT("中止しました。\r\n"));
			isReady = FALSE;
			while (!InvalidateRect(hMainWnd, &BanPaint, TRUE));
			return FALSE;
		}
	} while (TRUE);
}

INT_PTR CALLBACK MyDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, TEXT("n"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, szLevel, sizeof(szLevel) / sizeof(TCHAR) - 1);
			EndDialog(hDlg, IDOK);
			return 1;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return 1;
		}
		return 0;
	}
	return 0;
}

void MessageAdd(HWND hEdit, LPTSTR maadd)
{
	int strsize = 1, maaddsize;
	HGLOBAL hTmp = NULL;
	LPTSTR strTmp = NULL;

	if (lpszBuf != NULL)
		strsize = lstrlen(lpszBuf);
	maaddsize = lstrlen(maadd);
	hTmp = GlobalAlloc(GHND, (strsize + 1) * sizeof(TCHAR));
	strTmp = (LPTSTR)GlobalLock(hTmp);
	if (lpszBuf != NULL)
		lstrcpy(strTmp, lpszBuf);
	if (hStrMem != NULL) {
		GlobalUnlock(hStrMem);
		GlobalFree(hStrMem);
	}
	if (strsize + maaddsize < MESSAGE_MAX) {
		hStrMem = GlobalAlloc(GHND, (maaddsize + strsize + 1) * sizeof(TCHAR));
		lpszBuf = (LPTSTR)GlobalLock(hStrMem);
	}
	else {
		hStrMem = GlobalAlloc(GHND, MESSAGE_MAX * sizeof(TCHAR));
		lpszBuf = (LPTSTR)GlobalLock(hStrMem);
		strTmp[strsize - ((strsize + maaddsize + 1) - MESSAGE_MAX)] = L'\0';
	}
	lstrcpy(lpszBuf, maadd);
	lstrcat(lpszBuf, strTmp);
	GlobalUnlock(hTmp);
	GlobalFree(hTmp);

	SetWindowText(hEdit, lpszBuf);

	return;
}

void ChangeMode(HMENU hMenu, WORD idm, MENUITEMINFO* mii) {
	int id, i, j;
	BOOL isChange = FALSE;

	switch (idm) {
	case IDM_ANALYZE:
		ShowWindow(hButton[N + 1], SW_SHOW);
		ShowWindow(hButton[N + 2], SW_HIDE);
		ShowWindow(hButton[N + 3], SW_HIDE);
		EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_GRAYED);
		if (sizemode == SMD_SMALL)
			ChangeSizeMode(hMenu, mii);
		if (mode == MD_SOLVE) {
			SendMessage(hMainWnd, WM_COMMAND, IDM_SMALLDEL, 0);
			mii->dwTypeData = TEXT("出力結果を全削除(&O)");
			mii->cch = lstrlen(TEXT("出力結果を全削除(&O)")) + 1;
			SetMenuItemInfo(hMenu, IDM_OUTPUTDEL, FALSE, mii);
		}
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (Display[i][j] && !isInput[i][j]) {
					isChange = TRUE;
					break;
				}
			}
			if (isChange)
				break;
		}
		if (isChange) {
			id = MessageBox(hMainWnd,
				TEXT("盤面の数字を全て解析対象にしますか？"),
				TEXT("確認"), MB_YESNO);
			if (id == IDYES) {
				SendMessage(hMainWnd, WM_COMMAND, IDM_SAVEINPUT, 0);
			}
		}
		MessageAdd(hEdit, TEXT("解析モード：解析したい問題を入力してください。\r\n"));
		mode = MD_ANALYZE;
		break;
	case IDM_GENERATE:
		ShowWindow(hButton[N + 1], SW_HIDE);
		ShowWindow(hButton[N + 2], SW_SHOW);
		ShowWindow(hButton[N + 3], SW_HIDE);
		EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_GRAYED);
		if (sizemode == SMD_SMALL)
			ChangeSizeMode(hMenu, mii);
		if (mode == MD_SOLVE) {
			SendMessage(hMainWnd, WM_COMMAND, IDM_SMALLDEL, 0);
			mii->dwTypeData = TEXT("出力結果を全削除(&O)");
			mii->cch = lstrlen(TEXT("出力結果を全削除(&O)")) + 1;
			SetMenuItemInfo(hMenu, IDM_OUTPUTDEL, FALSE, mii);
		}
		MessageAdd(hEdit, TEXT("生成モード：生成する問題の一部分を入力により指定できます。\r\n"));
		mode = MD_GENERATE;
		break;
	case IDM_SOLVE:
		ShowWindow(hButton[N + 1], SW_HIDE);
		ShowWindow(hButton[N + 2], SW_HIDE);
		ShowWindow(hButton[N + 3], SW_SHOW);
		EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_ENABLED);
		mii->dwTypeData = TEXT("解答を全削除(&O)");
		mii->cch = lstrlen(TEXT("解答を全削除(&O)")) + 1;
		SetMenuItemInfo(hMenu, IDM_OUTPUTDEL, FALSE, mii);
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (Display[i][j] && !isInput[i][j]) {
					isChange = TRUE;
					break;
				}
			}
			if (isChange)
				break;
		}
		if (isChange) {
			if (mode == MD_ANALYZE) {
				id = MessageBox(hMainWnd,
					TEXT("問題の数字以外をすべて消去しますか？"),
					TEXT("確認"), MB_YESNO);
				if (id == IDYES) {
					SendMessage(hMainWnd, WM_COMMAND, IDM_OUTPUTDEL, 0);
				}
			}
			else if (mode == MD_GENERATE) {
				id = MessageBox(hMainWnd,
					TEXT("現在の盤面を問題にしますか？"),
					TEXT("確認"), MB_YESNO);
				if (id == IDYES) {
					SendMessage(hMainWnd, WM_COMMAND, IDM_SAVEINPUT, 0);
				}
			}
		}
		MessageAdd(hEdit, TEXT("解答モード：生成した問題に実際に挑戦します。\r\n"));
		SendMessage(hMainWnd, WM_COMMAND, ID_BUTTON_SOLVE, 0);
		mode = MD_SOLVE;
		break;
	}

	return;
}

void ChangeSizeMode(HMENU hMenu, MENUITEMINFO* mii) {
	switch (sizemode) {
	case SMD_BIG:
		sizemode = SMD_SMALL;
		mii->dwTypeData = TEXT("標準文字入力に変更(&S)");
		mii->cch = lstrlen(TEXT("標準文字入力に変更(&S)")) + 1;
		SetMenuItemInfo(hMenu, IDM_SIZECHANGE, FALSE, mii);
		MessageAdd(hEdit, TEXT("小文字入力に変更します。\r\n"));
		break;
	case SMD_SMALL:
		sizemode = SMD_BIG;
		mii->dwTypeData = TEXT("小文字入力に変更(&S)");
		mii->cch = lstrlen(TEXT("小文字入力に変更(&S)")) + 1;
		SetMenuItemInfo(hMenu, IDM_SIZECHANGE, FALSE, mii);
		MessageAdd(hEdit, TEXT("標準文字入力に変更します。\r\n"));
		break;
	}
	InvalidateRect(hMainWnd, &BanPaint, TRUE);

	return;
}

void ExitSolveThCheck() {
	if (ThEnd) {
		mbisfirst = TRUE;
		switch (mode) {
		case MD_ANALYZE:
			SetWindowText(hButton[N + 1], TEXT("解析！"));
			MessageAdd(hEdit, TEXT("中止しました。\r\n"));
			break;
		case MD_GENERATE:
			SetWindowText(hButton[N + 2], TEXT("問題生成"));
			if (TotalTimeOut) {
				MessageAdd(hEdit, TEXT("タイムアウトしました。処理を中断します。\r\n"));
			}
			else {
				MessageAdd(hEdit, TEXT("中止しました。\r\n"));
			}
			break;
		case MD_SOLVE:
			SetWindowText(hButton[10], TEXT("答え合わせ"));
			MessageAdd(hEdit, TEXT("中止しました。\r\n"));
			break;
		}
		BeforeThreadEnd();
		ExitThread(1);
	}

	return;
}

void BeforeThreadEnd()
{
	int i, j;

	isSolving = FALSE;
	if (!ThEnd && !TotalTimeOut) {
		if (mode != MD_SOLVE) {
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					Display[i][j] = BanAnalyze[i][j];
				}
			}
		}
	}
	while (!InvalidateRect(hMainWnd, &BanPaint, TRUE));
	for (i = 0; i < N + 1; i++) {
		EnableWindow(hButton[i], TRUE);
	}
	MenuSwitch(TRUE);

	return;
}

void MenuSwitch(BOOL onoff)
{
	if (onoff) {
		switch (mode) {
		case MD_ANALYZE:
			EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_OUTPUTDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_GRAYED);
			break;
		case MD_GENERATE:
			EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_OUTPUTDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_GRAYED);
			break;
		case MD_SOLVE:
			EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_OUTPUTDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_ENABLED);
			break;
		}
	}
	else {
		EnableMenuItem(hMenu, IDM_SAVEINPUT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_OUTPUTDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SMALLDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ALLDEL, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SIZECHANGE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_ANALYZE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_GENERATE, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
	}
	return;
}

BOOL AfterThreadStart()
{
	int i, j;
	BOOL isSolved;

	isSolving = TRUE;

	for (i = 0; i < N + 1; i++) {
		EnableWindow(hButton[i], FALSE);
	}

	MenuSwitch(FALSE);

	isSolved = DisToBanana();

	switch (mode) {
	case MD_ANALYZE:
		SetWindowText(hButton[N + 1], TEXT("中止"));
		MessageAdd(hEdit, TEXT("解析しています…\r\n"));
		break;
	case MD_SOLVE:
		SetWindowText(hButton[N + 3], TEXT("中止"));
		if (!isSolved) {
			MessageAdd(hEdit, TEXT("問題を判定しています…\r\n"));
		}
		else {
			MessageAdd(hEdit, TEXT("答え合わせしています…\r\n"));
		}
		break;
	}

	InvalidateRect(hMainWnd, &BanPaint, TRUE);

	TotalTime = 0;
	TotalTimeOut = FALSE;
	CountUp = 0;

	return isSolved;
}

BOOL DisToBanana()
{
	int i, j;
	BOOL isSolved = FALSE;

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (isInput[i][j])
				BanAnalyze[i][j] = Display[i][j];
			else {
				BanAnalyze[i][j] = 0;
				if (Display[i][j]) {
					isSolved = TRUE;
				}
			}
		}
	}

	return isSolved;
}

void WINAPI Analyze()
{
	int i, j, cb, id;

	AfterThreadStart();

	cb = CheckBan(hMainWnd);

	switch (cb) {
	case CB_END:
		ExitSolveThCheck();
		return;
	case CB_FINISH:
		for (i = 0; i < N; i++)
			for (j = 0; j < N; j++)
				BanAnalyze[i][j] = Stock[i][j];
		if (hardness == 0) {
			MessageAdd(hEdit, TEXT("すでに全てのマスが埋まっています！\r\n"));
		}
		else {
			wsprintf(add, TEXT("解析が完了しました。問題レベル：%d（%dポイント）\r\n"), (int)(2.0 * log10(hardness)), hardness);
			MessageAdd(hEdit, add);
		}
		break;
	case CB_DOUBLE:
		id = MessageBox(hMainWnd,
			TEXT("複数の解答パターンを見つけました。\nその内の１つを表示しますか？"),
			TEXT("確認"), MB_YESNO);
		if (id == IDYES) {
			for (i = 0; i < N; i++)
				for (j = 0; j < N; j++)
					BanAnalyze[i][j] = Stock[i][j];
			MessageAdd(hEdit, TEXT("解答パターンのうち１つを表示します。\r\n"));
		}
		else if (id == IDNO) {
			for (i = 0; i < N; i++)
				for (j = 0; j < N; j++)
					BanAnalyze[i][j] = Display[i][j];
			MessageAdd(hEdit, TEXT("中止しました。\r\n"));
		}
		break;
	case CB_NIL:
		for (i = 0; i < N; i++)
			for (j = 0; j < N; j++)
				BanAnalyze[i][j] = Display[i][j];
	case CB_ERROR:
		MessageAdd(hEdit, TEXT("こんな盤面は解けません！\r\n"));
		break;
	}

	SetWindowText(hButton[N + 1], TEXT("解析！"));
	BeforeThreadEnd();

	return;
}

void WINAPI Generate()
{
	int result;

	AfterThreadStart();

	result = MakeBan();

	switch (result) {
	case MB_ERROR:
		MessageAdd(hEdit, TEXT("入力した盤面に誤りがあります。\r\n"));
		break;
	case MB_ZERO:
		MessageAdd(hEdit, TEXT("すでに全てのマスが埋まっています！\r\n"));
		break;
	case MB_IMPOSSIBLE:
		MessageAdd(hEdit, TEXT("入力した問題レベルを実現できない盤面です。\r\n"));
		break;
	default:
		wsprintf(add, TEXT("作成成功！問題レベル：%d（%dポイント）\r\n"), (int)(2.0 * log10(result)), result);
		MessageAdd(hEdit, add);
		break;
	}

	SetWindowText(hButton[N + 2], TEXT("問題生成"));
	BeforeThreadEnd();

	return;
}

void WINAPI Solve()
{
	int cb, i, j, Qhardness;
	BOOL isSolved;

	isSolved = AfterThreadStart();

	cb = CheckBan(hMainWnd);

	Qhardness = hardness;

	switch (cb) {
	case CB_FINISH:
		if (isSolved) {
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					BanAnalyze[i][j] = Display[i][j];
				}
			}
			switch (CheckBan(hMainWnd)) {
			case CB_FINISH:
				if (hardness == 0) {
					wsprintf(add, TEXT("判定：正解！おめでとう！問題レベル：%d（%dポイント）\r\n"), (int)(2.0 * log10(Qhardness)), Qhardness);
					MessageAdd(hEdit, add);
				}
				else {
					wsprintf(add, TEXT("判定：ここまで合っています。残りの問題レベル：%d（%dポイント）\r\n"), (int)(2.0 * log10(hardness)), hardness);
					MessageAdd(hEdit, add);
				}
				break;
			default:
				MessageAdd(hEdit, TEXT("判定：間違いがあります。\r\n"));
			}
		}
		else {
			if (hardness == 0) {
				MessageAdd(hEdit, TEXT("判定：この問題はすでに全てのマスが埋まっています。\r\n"));
			}
			else {
				wsprintf(add, TEXT("判定：この問題は解けます。問題レベル：%d（%dポイント）\r\n"), (int)(2.0 * log10(hardness)), hardness);
				MessageAdd(hEdit, add);
			}
		}
		break;
	default:
		MessageAdd(hEdit, TEXT("判定：この問題は解けません。\r\n"));
		break;
	}

	SetWindowText(hButton[N + 3], TEXT("答え合わせ"));
	BeforeThreadEnd();

	return;
}

int MakeBan()
{
	int result, fib, i, j;

	isDrilling = FALSE;

	if (notarget) {
		return FillInBan(FALSE);
	}
	else {
		do {
			DisToBanana();

			if ((fib = FillInBan(TRUE)) != FIB_NULL) {
				return fib;
			}

			isDrilling = TRUE;
			if ((result = DrillBan()) >= 0)
				return result;
			isDrilling = FALSE;
		} while (TRUE);
	}
}

int FillInBan(BOOL isfillin)
{
	BOOL uds[N], skip0 = FALSE, skip1 = FALSE, isfirstfunc = FALSE;
	static BOOL uups[N][N];
	int i, j, giverv = FIB_DEFAULT, udn, r1, r2, digit, place, nuups[N * N], nuupn = 0, cb, curhard;
	static int uupn;

	if (mbisfirst) {
		mbisfirst = FALSE;
		isfirstfunc = TRUE;
	}

	ExitSolveThCheck();

	if (isfirstfunc) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				SubAnalyze[i][j] = BanAnalyze[i][j];
			}
		}
		uupn = 0;
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (isInput[i][j]) {
					uupn++;
					uups[i][j] = TRUE;
				}
				else {
					uups[i][j] = FALSE;
				}
			}
		}
	}

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			BanAnalyze[i][j] = SubAnalyze[i][j];
		}
	}

	cb = CheckBan(hMainWnd);

	curhard = hardness;

	while (TRUE) {
		switch (cb) {
		case CB_FINISH:
			if (isfirstfunc) {
				if (isfirstfunc && !curhard) {
					giverv = MB_ZERO;
					skip0 = skip1 = TRUE;
					break;
				}
				else if (isfillin && curhard < pow(10.0, target / 2.0)) {
					giverv = MB_IMPOSSIBLE;
					skip0 = skip1 = TRUE;
					break;
				}
				else if (isfillin && curhard >= pow(10.0, target / 2.0) && curhard < pow(10.0, (target + 1) / 2.0)) {
					giverv = curhard;
					skip0 = TRUE;
					break;
				}
				else if (!isfillin) {
					giverv = curhard;
					skip0 = TRUE;
					break;
				}
				else {
					for (i = 0; i < N; i++)
						for (j = 0; j < N; j++)
							BanAnalyze[i][j] = Stock[i][j];
					giverv = FIB_NULL;
					skip0 = skip1 = TRUE;
					break;
				}
			}
			else if (isfillin) {
				if (curhard >= pow(10.0, target / 2.0) && curhard < pow(10.0, (target + 1) / 2.0)) {
					giverv = curhard;
					skip0 = TRUE;
					break;
				}
				else {
					for (i = 0; i < N; i++)
						for (j = 0; j < N; j++)
							BanAnalyze[i][j] = Stock[i][j];
					giverv = FIB_NULL;
					skip0 = skip1 = TRUE;
					break;
				}
			}
			else {
				giverv = curhard;
				skip0 = TRUE;
				break;
			}
		case CB_DOUBLE:
			if (isfirstfunc) {
				if (isfillin && curhard < pow(10.0, target / 2.0)) {
					giverv = MB_IMPOSSIBLE;
					skip0 = skip1 = TRUE;
					break;
				}
			}
			break;
		case CB_NIL:
		case CB_ERROR:
		case CB_TIMEOUT:
			if (isfirstfunc) {
				giverv = MB_ERROR;
			}
			else
				giverv = FIB_ERROR;
			skip0 = TRUE;
		}
		break;
	}

	if (!skip1) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				BanAnalyze[i][j] = SubAnalyze[i][j];
			}
		}
	}

	if (!skip0) {
		srand((unsigned)time(NULL));
		for (nuupn = 0; uupn < N * N; nuupn++) {
			r2 = rand() % (N * N - uupn) + 1;
			place = -1;
			for (j = 0; j < r2; j++) {
				place++;
				while (uups[place / N][place % N]) {
					place++;
				}
			}
			uupn++;
			uups[place / N][place % N] = TRUE;
			nuups[nuupn] = place;
			udn = 0;
			for (i = 0; i < N; i++) {
				uds[i] = FALSE;
			}
			while (udn < N) {
				r1 = rand() % (N - udn) + 1;
				digit = 0;
				for (j = 0; j < r1; j++) {
					digit++;
					while (uds[digit - 1]) {
						digit++;
					}
				}

				udn++;
				uds[digit - 1] = TRUE;
				BanAnalyze[place / N][place % N] = SubAnalyze[place / N][place % N] = digit;

				giverv = FillInBan(isfillin);

				ExitSolveThCheck();

				hardness = curhard;

				if (giverv >= 0) {
					break;
				}
			}

			if (giverv >= 0) {
				break;
			}
			BanAnalyze[place / N][place % N] = SubAnalyze[place / N][place % N] = 0;
		}
		if (giverv < 0 && uupn >= N * N) {
			if (isfirstfunc)
				giverv = MB_ERROR;
			else
				giverv = FIB_ERROR;
		}

		if (giverv < 0) {
			uupn -= nuupn;
			for (i = 0; i < nuupn; i++) {
				uups[nuups[i] / N][nuups[i] % N] = FALSE;
			}
		}
	}

	if (isfirstfunc) {
		mbisfirst = TRUE;
	}

	return giverv;
}

int DrillBan()
{
	BOOL skip = FALSE, skip0 = FALSE, isfirstfunc = FALSE;
	static BOOL udps[N][N];
	int i, j, giverv = DB_DEFAULT, r, digit, place, nudps[N * N], nudpn = 0, cb, curhard, parhard;
	static int udpn;
	double distance;

	if (mbisfirst) {
		mbisfirst = FALSE;
		isfirstfunc = TRUE;
	}

	ExitSolveThCheck();

	if (isfirstfunc) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				SubAnalyze[i][j] = BanAnalyze[i][j];
			}
		}
		udpn = 0;
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (isInput[i][j]) {
					udps[i][j] = TRUE;
					udpn++;
				}
				else
					udps[i][j] = FALSE;
			}
		}
	}

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			BanAnalyze[i][j] = SubAnalyze[i][j];
		}
	}

	if (!isfirstfunc)
		parhard = hardness;

	cb = CheckBan(hMainWnd);

	curhard = hardness;
	if (isfirstfunc)
		parhard = curhard;
	if(curhard != 0 && parhard != 0)
		distance = log10(curhard) - log10(parhard);

	while (TRUE) {
		switch (cb) {
		case CB_FINISH:
			if (curhard == 0 || parhard == 0) {
				break;
			}
			else if (curhard >= pow(10.0, target / 2.0) && curhard < pow(10.0, (target + 1) / 2.0)) {
				giverv = curhard;
				skip0 = TRUE;
			}
			else if (pow(10.0, log10(curhard) + distance) > pow(10.0, (target + 1) / 2.0)) {
				giverv = DB_ERROR;
				skip0 = TRUE;
			}
			break;
		case CB_DOUBLE:
		case CB_NIL:
		case CB_ERROR:
		case CB_TIMEOUT:
			giverv = DB_ERROR;
			skip0 = TRUE;
			break;
		}
		break;
	}

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			BanAnalyze[i][j] = SubAnalyze[i][j];
		}
	}

	if (!skip0) {
		srand((unsigned)time(NULL));
		for (nudpn = 0; udpn < N * N; nudpn++) {
			r = rand() % (N * N - udpn) + 1;
			place = -1;
			for (j = 0; j < r; j++) {
				place++;
				while (udps[place / N][place % N]) {
					place++;
				}
			}
			udpn++;
			udps[place / N][place % N] = TRUE;
			nudps[nudpn] = place;
			digit = SubAnalyze[place / N][place % N];
			BanAnalyze[place / N][place % N] = SubAnalyze[place / N][place % N] = 0;

			giverv = DrillBan();

			ExitSolveThCheck();

			hardness = curhard;

			if (giverv >= 0)
				break;
			BanAnalyze[place / N][place % N] = SubAnalyze[place / N][place % N] = digit;
		}
		if (giverv < 0 && udpn >= N * N)
			giverv = DB_ERROR;

		if (giverv < 0) {
			udpn -= nudpn;
			for (i = 0; i < nudpn; i++) {
				udps[nudps[i] / N][nudps[i] % N] = FALSE;
			}
		}
	}

	if (isfirstfunc) {
		mbisfirst = TRUE;
	}

	return giverv;
}

int CheckBan(HWND hWnd)
{
	BOOL isSolvable = FALSE;

	hardness = 0;
	ExRem = FALSE;
	DSolution = FALSE;
	TimeOut = FALSE;
	TryCount = 0;
	CanEscape = FALSE;

	ExitSolveThCheck();

	isSolvable = CheckSolvable();

	if (isSolvable) {
		Try();
		if (TimeOut)
			return CB_TIMEOUT;
		if (ExRem) {
			if (!DSolution) {
				return CB_FINISH;
			}
			else {
				return CB_DOUBLE;
			}
		}
		else {
			return CB_NIL;
		}
	}
	else {
		if (TimeOut)
			return CB_TIMEOUT;
		return CB_ERROR;
	}
}

void Try() {
	int x, y, k, i, j;
	static BOOL TryEnd, tryisfirst = TRUE;
	BOOL isfirstfunc = FALSE;

	if (tryisfirst) {
		tryisfirst = FALSE;
		isfirstfunc = TRUE;
	}

	if (isfirstfunc) {
		TryEnd = FALSE;
	}

	TryCount = (TryCount + 1) % (ONE_WAIT_TIME * TIME_UNIT);
	if (!TryCount) {
		if (mode == MD_GENERATE) {
			TimeOut = TRUE;
			TryEnd = TRUE;
		}
	}
	TotalTime = (TotalTime + 1) % (ONE_WAIT_TIME * TIME_UNIT);
	if (!TotalTime) {
		if (!CountUp) {
			MessageAdd(hEdit, TEXT("|\r\n"));
		}
		else {
			MessageAdd(hEdit, TEXT("|"));
		}
		CountUp++;
	}
	if (CountUp >= WAIT_TIME) {
		TotalTimeOut = TRUE;
		ThEnd = TRUE;
	}

	ExitSolveThCheck();
	if (EndTryCheck(&TryEnd, &tryisfirst)) {
		return;
	}

	if (FindBrank(&x, &y)) {
		for (k = 1; k <= N; k++) {
			if (isOkeru(x, y, k)) {
				BanAnalyze[y][x] = k;
				hardness++;
				if (!notarget && mode == MD_GENERATE) {
					if (isDrilling) {
						if (hardness > pow(10.0, (target + 1) / 2.0)) {
							TryEnd = TRUE;
						}
					}
					else {
						if(hardness > pow(10.0, (target + 1) / 2.0) && DSolution)
							TryEnd = TRUE;
					}
				}
				Try();

				ExitSolveThCheck();
				if (EndTryCheck(&TryEnd, &tryisfirst)) {
					return;
				}

				BanAnalyze[y][x] = 0;
			}
		}
	}
	else {
		if (!ExRem) {
			for (i = 0; i < N; i++)
				for (j = 0; j < N; j++)
					Stock[i][j] = BanAnalyze[i][j];
			ExRem = TRUE;
		}
		else {
			DSolution = TRUE;
			if(!(mode == MD_GENERATE && !notarget))
				TryEnd = TRUE;
		}
	}

	if (isfirstfunc) {
		tryisfirst = TRUE;
	}

	return;
}

BOOL EndTryCheck(BOOL* TryEnd, BOOL* tryisfirst) {
	if (*TryEnd) {
		*tryisfirst = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL FindBrank(int* x, int* y)
{
	int i, j;
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			if (!BanAnalyze[i][j])
			{
				*y = i; *x = j;
				return TRUE;
			}
	return FALSE;
}

int isOkeru(int x, int y, int k)
{
	int i, j;

	for (i = 0; i < N; i++)
		if (BanAnalyze[i][x] == k)
			return FALSE;
	for (j = 0; j < N; j++)
		if (BanAnalyze[y][j] == k)
			return FALSE;
	for (i = 0; i < N2; i++)
		for (j = 0; j < N2; j++)
			if (BanAnalyze[N2 * (y / N2) + i][N2 * (x / N2) + j] == k)
				return FALSE;
	return TRUE;
}

BOOL CheckSolvable(void)
{
	int x, y, tmp;

	for (y = 0; y < N; y++) {
		for (x = 0; x < N; x++) {
			if (BanAnalyze[y][x] != 0) {
				tmp = BanAnalyze[y][x];
				BanAnalyze[y][x] = 0;
				if (!isOkeru(x, y, tmp)) {
					BanAnalyze[y][x] = tmp;
					return FALSE;
				}
				BanAnalyze[y][x] = tmp;
			}
		}
	}

	return TRUE;
}
