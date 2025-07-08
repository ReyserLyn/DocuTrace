import { create } from 'zustand';
import { devtools } from 'zustand/middleware';

export type ViewType = 'home' | 'settings';

interface ViewState {
  currentView: ViewType;
  previousView: ViewType | null;
  setView: (view: ViewType) => void;
  goBack: () => void;
  canGoBack: () => boolean;
}

export const useViewStore = create<ViewState>()(
  devtools(
    (set, get) => ({
      currentView: 'home',
      previousView: null,

      setView: (view: ViewType) => {
        const current = get().currentView;
        set({ 
          currentView: view, 
          previousView: current !== view ? current : get().previousView 
        });
      },

      goBack: () => {
        const { previousView } = get();
        if (previousView) {
          set({ 
            currentView: previousView, 
            previousView: null 
          });
        }
      },

      canGoBack: () => {
        return get().previousView !== null;
      },
    }),
    {
      name: 'view-store',
    }
  )
); 