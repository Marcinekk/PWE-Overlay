interface Transaction {
    id: number;
    timestamp: number;
    amount: number;
    income: boolean;
    type: string;
    description: string;
    status: 'pending' | 'approved' | 'rejected';
}

export type { Transaction };